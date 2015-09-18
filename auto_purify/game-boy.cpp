void AutoPurify::copyGameBoySaves(const string &pathname) {
  if(!file::exists({pathname, "save.ram"})) {
    if(file::exists({information.path, prefixname(information.name), ".sav"})) {
      file::copy({information.path, prefixname(information.name), ".sav"}, {pathname, "save.ram"});
    }
  }

  if(!file::exists({pathname, "rtc.ram"})) {
    if(file::exists({information.path, prefixname(information.name), ".rtc"})) {
      file::copy({information.path, prefixname(information.name), ".rtc"}, {pathname, "rtc.ram"});
    }
  }
}

string AutoPurify::createGameBoyHeuristic(vector<uint8_t> &buffer) {
  GameBoyCartridge info(buffer.data(), buffer.size());

  string pathname = {
    libraryPath, "Game Boy", (info.info.cgb ? " Color" : ""), "/",
    prefixname(information.name),
    ".", (info.info.cgb ? "gbc" : "gb"), "/"
  };
  directory::create(pathname);

  string markup = {"unverified\n\n", info.markup};
  markup.append("\ninformation\n  title: ", prefixname(information.name), "\n");
  if(!information.manifest.empty()) markup = information.manifest;  //override with embedded beat manifest, if one exists

  file::write({pathname, "manifest.bml"}, markup);
  file::write({pathname, "program.rom"}, buffer);

  copyGameBoySaves(pathname);
  return pathname;
}

string AutoPurify::openGameBoy(vector<uint8_t> &buffer) {
  return createGameBoyHeuristic(buffer);
}

string AutoPurify::syncGameBoy(const string &pathname) {
  auto buffer = file::read({pathname, "program.rom"});
  if(buffer.size() == 0) return "";

  auto save = file::read({pathname, "save.ram"});
  if(save.size() == 0) save = file::read({pathname, "save.rwm"});

  auto rtc = file::read({pathname, "rtc.ram"});
  if(rtc.size() == 0) rtc = file::read({pathname, "rtc.rwm"});

  directory::remove(pathname);
  information.path = pathname;
  information.name = basename(pathname).rtrim("/", 1L);
  string outputPath = openGameBoy(buffer);

  if(save.size()) file::write({outputPath, "save.ram"}, save);
  if(rtc.size()) file::write({outputPath, "rtc.ram"}, rtc);

  return outputPath;
}
