namespace WonderSwan {

struct ID {
  enum : uint {
    System,
    WonderSwan,
    WonderSwanColor,
  };

  struct Port { enum : uint {
    Hardware,
  };};

  struct Device { enum : uint {
    HorizontalControls,
    VerticalControls,
  };};
};

struct WonderSwanInterface : Emulator::Interface {
  using Emulator::Interface::load;

  WonderSwanInterface();

  auto manifest() -> string override;
  auto title() -> string override;

  auto videoSize() -> VideoSize override;
  auto videoSize(uint width, uint height, bool arc, bool intScale) -> VideoSize override;
  auto videoFrequency() -> double override;
  auto videoColors() -> uint32;
  auto videoColor(uint32 color) -> uint64;

  auto audioFrequency() -> double override;

  auto loaded() -> bool override;
  auto sha256() -> string override;
  auto load(uint id) -> bool override;
  auto save() -> void override;
  auto unload() -> void override;

  auto connect(uint port, uint device) -> void;
  auto power() -> void override;
  auto run() -> void override;
  auto rotate() -> void override;

  auto serialize() -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto cheatSet(const string_vector&) -> void override;

  auto cap(const string& name) -> bool override;
  auto get(const string& name) -> any override;
  auto set(const string& name, const any& value) -> bool override;
};

struct WonderSwanColorInterface : Emulator::Interface {
  using Emulator::Interface::load;

  WonderSwanColorInterface();

  auto manifest() -> string override;
  auto title() -> string override;

  auto videoSize() -> VideoSize override;
  auto videoSize(uint width, uint height, bool arc, bool intScale) -> VideoSize override;
  auto videoFrequency() -> double override;
  auto videoColors() -> uint32;
  auto videoColor(uint32 color) -> uint64;

  auto audioFrequency() -> double override;

  auto loaded() -> bool override;
  auto sha256() -> string override;
  auto load(uint id) -> bool override;
  auto save() -> void override;
  auto unload() -> void override;

  auto connect(uint port, uint device) -> void;
  auto power() -> void override;
  auto run() -> void override;
  auto rotate() -> void override;

  auto serialize() -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto cheatSet(const string_vector&) -> void override;

  auto cap(const string& name) -> bool override;
  auto get(const string& name) -> any override;
  auto set(const string& name, const any& value) -> bool override;
};

struct Settings {
  bool blurEmulation = true;
  bool colorEmulation = true;
};

extern Settings settings;

}
