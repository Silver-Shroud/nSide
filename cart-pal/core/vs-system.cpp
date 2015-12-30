auto CartPal::vsSystemManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "ines.rom"});
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "character.rom"});
  concatenate(buffer, {location, "program-1.rom"});
  concatenate(buffer, {location, "character-1.rom"});
  concatenate(buffer, {location, "program-2.rom"});
  concatenate(buffer, {location, "character-2.rom"});
  return vsSystemManifest(buffer, location);
}

auto CartPal::vsSystemManifest(vector<uint8_t>& buffer, string location, uint* prgrom, uint* chrrom) -> string {
  string markup;
  unsigned offset = (buffer.size() & 0x1fff) == 0 ? 0 : 16;
  if(buffer.data()[0] != 'N'
  || buffer.data()[1] != 'E'
  || buffer.data()[2] != 'S'
  || buffer.data()[3] !=  26) offset = 0;
  string digest = Hash::SHA256(buffer.data() + offset, buffer.size() - offset).digest();

  if(settings["cart-pal/UseDatabase"].boolean() && !markup) {
    for(auto node : database.vsSystem) {
      if(node["sha256"].text() == digest) {
        markup.append(node.text(), "\n  sha256:   ", digest, "\n");
        break;
      }
    }
  }

  if(settings["cart-pal/UseHeuristics"].boolean() && !markup) {
    FamicomCartridge cartridge{buffer.data(), buffer.size()};
    if(auto markup = cartridge.markup) {
      markup.append("\n");
      markup.append("information\n");
      markup.append("  title:  ", prefixname(location), "\n");
      markup.append("  sha256: ", digest, "\n");
      markup.append("  note:   ", "heuristically generated by cart-pal\n");
    }
  }

  auto document = BML::unserialize(markup);
  if(prgrom) *prgrom = document["side/prg/rom/size"].natural();  //0 if node does not exist
  if(chrrom) *chrrom = document["side/chr/rom/size"].natural();  //0 if node does not exist

  return markup;
}

auto CartPal::vsSystemImport(vector<uint8_t>& buffer, string location) -> bool {
  bool has_ines_header = true;
  if(buffer.data()[0] != 'N'
  || buffer.data()[1] != 'E'
  || buffer.data()[2] != 'S'
  || buffer.data()[3] !=  26) has_ines_header = false;
  unsigned offset = has_ines_header ? 16 : 0;

  auto name = prefixname(location);
  auto source = pathname(location);
  string target{settings["Library/Location"].text(), "VS. System/", name, ".vs/"};
//if(directory::exists(target)) return failure("game already exists");

  string markup;
  vector<Markup::Node> roms;

  if(settings["cart-pal/UseDatabase"].boolean() && !markup) {
    auto digest = Hash::SHA256(buffer.data() + offset, buffer.size() - offset).digest();
    for(auto node : database.vsSystem) {
      if(node.name() != "release") continue;
      if(node["information/sha256"].text() == digest) {
        markup.append(BML::serialize(node["cartridge"]), "\n");
        markup.append(BML::serialize(node["information"]));
        break;
      }
    }
  }

  if(!markup) roms.append(BML::unserialize("rom name=ines.rom size=0x10")["rom"]);

  if(settings["cart-pal/UseHeuristics"].boolean() && !markup) {
    offset = 0;
    FamicomCartridge cartridge{buffer.data(), buffer.size()};
    if(markup = cartridge.markup) {
      markup.append("\n");
      markup.append("information\n");
      markup.append("  title: ", name, "\n");
      markup.append("  note:  heuristically generated by cart-pal\n");
    }
  }

  auto document = BML::unserialize(markup);
  vsSystemImportScanManifest(roms, document["cartridge"]);

  if(!markup) return failure("failed to parse ROM image");
  if(!directory::create(target)) return failure("library path unwritable");

  if(settings["cart-pal/CreateManifests"].boolean()) file::write({target, "manifest.bml"}, markup);
  for(auto rom : roms) {
    auto name = rom["name"].text();
    auto size = rom["size"].natural();
    if(size > buffer.size() - offset) return failure("ROM image is missing data");
    file::write({target, name}, buffer.data() + offset, size);
    offset += size;
  }
  return success();
}

auto CartPal::vsSystemImportScanManifest(vector<Markup::Node>& roms, Markup::Node node) -> void {
  if(node.name() == "rom") roms.append(node);
  for(auto leaf : node) vsSystemImportScanManifest(roms, leaf);
}
