struct CartPal {
  //core.cpp
  CartPal();

  auto error() const -> string;
  auto success() -> bool;
  auto failure(string message) -> bool;

  auto manifest(string location) -> string;
  auto import(string location) -> bool;

  auto concatenate(vector<uint8>& output, string location) -> void;

  //famicom.cpp
  //auto famicomManifest(string location) -> string;
  //auto famicomManifest(vector<uint8>& buffer, string location, uint* prgrom = nullptr, uint* chrrom = nullptr) -> string;
  //auto famicomImport(vector<uint8>& buffer, string location) -> bool;

  //super-famicom.cpp
  auto superFamicomManifest(string location) -> string;
  auto superFamicomManifest(vector<uint8>& buffer, string location, bool* firmwareAppended = nullptr) -> string;
  auto superFamicomManifestScan(vector<Markup::Node>& roms, Markup::Node node) -> void;
  auto superFamicomImport(vector<uint8>& buffer, string location) -> bool;

  //game-boy.cpp
  auto gameBoyManifest(string location) -> string;
  auto gameBoyManifest(vector<uint8>& buffer, string location) -> string;
  auto gameBoyImport(vector<uint8>& buffer, string location) -> bool;

  //game-boy-color.cpp
  auto gameBoyColorManifest(string location) -> string;
  auto gameBoyColorManifest(vector<uint8>& buffer, string location) -> string;
  auto gameBoyColorImport(vector<uint8>& buffer, string location) -> bool;

  //game-boy-advance.cpp
  auto gameBoyAdvanceManifest(string location) -> string;
  auto gameBoyAdvanceManifest(vector<uint8>& buffer, string location) -> string;
  auto gameBoyAdvanceImport(vector<uint8>& buffer, string location) -> bool;

  //bs-memory.cpp
  auto bsMemoryManifest(string location) -> string;
  auto bsMemoryManifest(vector<uint8>& buffer, string location) -> string;
  auto bsMemoryImport(vector<uint8>& buffer, string location) -> bool;

  //sufami-turbo.cpp
  auto sufamiTurboManifest(string location) -> string;
  auto sufamiTurboManifest(vector<uint8>& buffer, string location) -> string;
  auto sufamiTurboImport(vector<uint8>& buffer, string location) -> bool;

private:
  string errorMessage;

  struct {
    //Markup::Node famicom;
    Markup::Node superFamicom;
    Markup::Node gameBoy;
    Markup::Node gameBoyColor;
    Markup::Node gameBoyAdvance;
    Markup::Node bsMemory;
    Markup::Node sufamiTurbo;
  } database;
};
