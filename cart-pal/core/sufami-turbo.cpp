auto CartPal::sufamiTurboManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "program.rom"});
  return sufamiTurboManifest(buffer, location);
}

auto CartPal::sufamiTurboManifest(vector<uint8_t>& buffer, string location) -> string {
  string markup;
  string digest = Hash::SHA256(buffer.data(), buffer.size()).digest();

  if(settings["cart-pal/UseDatabase"].boolean() && !markup) {
    for(auto node : database.sufamiTurbo) {
      if(node["sha256"].text() == digest) {
        markup.append(node.text(), "\n  sha256:   ", digest, "\n");
        break;
      }
    }
  }

  if(settings["cart-pal/UseHeuristics"].boolean() && !markup) {
    SufamiTurboCartridge cartridge{buffer.data(), buffer.size()};
    if(markup = cartridge.markup) {
      markup.append("\n");
      markup.append("information\n");
      markup.append("  title:  ", Location::prefix(location), "\n");
      markup.append("  sha256: ", digest, "\n");
      markup.append("  note:   ", "heuristically generated by cart-pal\n");
    }
  }

  return markup;
}

auto CartPal::sufamiTurboImport(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "Sufami Turbo/", name, ".st/"};
//if(directory::exists(target)) return failure("game already exists");

  auto markup = sufamiTurboManifest(buffer, location);
  if(!markup) return failure("failed to parse ROM image");
  if(!directory::create(target)) return failure("library path unwritable");

  if(settings["cart-pal/CreateManifests"].boolean()) file::write({target, "manifest.bml"}, markup);
  file::write({target, "program.rom"}, buffer);
  return success(target);
}
