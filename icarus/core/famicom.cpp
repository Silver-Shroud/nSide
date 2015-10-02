auto Icarus::famicomManifest(const string& location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "ines.rom"});
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "character.rom"});
  return famicomManifest(buffer, location);
}

auto Icarus::famicomManifest(vector<uint8_t>& buffer, const string& location) -> string {
  FamicomCartridge cartridge{buffer.data(), buffer.size()};
  if(auto markup = cartridge.markup) {
    unsigned offset = (buffer.size() & 0x1fff) == 0 ? 0 : 16;
    if(buffer.data()[0] != 'N'
    || buffer.data()[1] != 'E'
    || buffer.data()[2] != 'S'
    || buffer.data()[3] !=  26) offset = 0;
    markup.append("\n");
    markup.append("information\n");
    markup.append("  sha256: ", Hash::SHA256(buffer.data() + offset, buffer.size() - offset).digest(), "\n");
    markup.append("  title:  ", prefixname(location), "\n");
    markup.append("  note:   ", "heuristically generated by icarus\n");
    return markup;
  }
  return "";
}

auto Icarus::famicomImport(vector<uint8_t>& buffer, const string& location) -> bool {
  bool has_ines_header = true;
  if(buffer.data()[0] != 'N'
  || buffer.data()[1] != 'E'
  || buffer.data()[2] != 'S'
  || buffer.data()[3] !=  26) has_ines_header = false;
  unsigned offset = has_ines_header ? 16 : 0;

  if(has_ines_header) {
    if(buffer.data()[7] & 0x01) return vsSystemImport(buffer, location);
    if((buffer.data()[7] & 0x0c) == 0x08) { // NES 2.0
      if(buffer.data()[7] & 0x02) return playchoice10Import(buffer, location);
    }
  }

  auto name = prefixname(location);
  auto source = pathname(location);
  string target{settings.libraryPath, "Famicom/", name, ".fc/"};
//if(directory::exists(target)) return failure("game already exists");

  string markup;
  vector<Markup::Node> roms;

  if(settings.useDatabase && !markup) {
    auto digest = Hash::SHA256(buffer.data() + offset, buffer.size() - offset).digest();
    for(auto node : database.famicom) {
      if(node.name() != "release") continue;
      if(node["information/sha256"].text() == digest) {
        markup.append(BML::serialize(node["cartridge"]), "\n");
        markup.append(BML::serialize(node["information"]));
        break;
      }
    }
  }

  if(!markup) roms.append(BML::unserialize("rom name=ines.rom size=0x10")["rom"]);

  if(settings.useHeuristics && !markup) {
    offset = 0;
    FamicomCartridge cartridge{buffer.data(), buffer.size()};
    if(markup = cartridge.markup) {
      markup.append("\n");
      markup.append("information\n");
      markup.append("  title: ", name, "\n");
      markup.append("  note:  heuristically generated by icarus\n");
    }
  }

  auto document = BML::unserialize(markup);
  famicomImportScanManifest(roms, document["cartridge"]);

  if(!markup) return failure("failed to parse ROM image");
  if(!directory::create(target)) return failure("library path unwritable");

  if(settings.createManifests) file::write({target, "manifest.bml"}, markup);
  for(auto rom : roms) {
    auto name = rom["name"].text();
    auto size = rom["size"].decimal();
    file::write({target, name}, buffer.data() + offset, size);
    offset += size;
  }
  return success();
}

auto Icarus::famicomImportScanManifest(vector<Markup::Node>& roms, Markup::Node node) -> void {
  if(node.name() == "rom") roms.append(node);
  for(auto leaf : node) famicomImportScanManifest(roms, leaf);
}
