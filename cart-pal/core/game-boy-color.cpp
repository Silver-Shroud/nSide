auto CartPal::gameBoyColorManifest(string location) -> string {
  vector<uint8> buffer;
  concatenate(buffer, {location, "program.rom"});
  return gameBoyColorManifest(buffer, location);
}

auto CartPal::gameBoyColorManifest(vector<uint8>& buffer, string location) -> string {
  string markup;
  string digest = Hash::SHA256(buffer.data(), buffer.size()).digest();

  if(settings["cart-pal/UseDatabase"].boolean() && !markup) {
    for(auto node : database.gameBoyColor) {
      if(node["sha256"].text() == digest) {
        markup.append(node.text(), "\n  sha256:   ", digest, "\n");
        break;
      }
    }
  }

  if(settings["cart-pal/UseHeuristics"].boolean() && !markup) {
    GameBoyCartridge cartridge{buffer.data(), buffer.size()};
    if(markup = cartridge.markup) {
      markup.append("\n");
      markup.append("information\n");
      markup.append("  title:  ", prefixname(location), "\n");
      markup.append("  sha256: ", digest, "\n");
      markup.append("  note:   ", "heuristically generated by cart-pal\n");
    }
  }

  return markup;
}

auto CartPal::gameBoyColorImport(vector<uint8>& buffer, string location) -> bool {
  auto name = prefixname(location);
  auto source = pathname(location);
  string target{settings["Library/Location"].text(), "Game Boy Color/", name, ".gbc/"};
//if(directory::exists(target)) return failure("game already exists");

  auto markup = gameBoyColorManifest(buffer, location);
  if(!markup) return failure("failed to parse ROM image");
  if(!directory::create(target)) return failure("library path unwritable");

  if(settings["cart-pal/CreateManifests"].boolean()) file::write({target, "manifest.bml"}, markup);
  file::write({target, "program.rom"}, buffer);
  return success();
}
