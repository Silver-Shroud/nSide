struct Memory {
  virtual inline auto size() const -> uint;
  virtual auto read(uint16 addr, uint8 data = 0) -> uint8 = 0;
  virtual auto write(uint16 addr, uint8 data) -> void = 0;
};

struct StaticRAM : Memory {
  inline StaticRAM(uint size);
  inline ~StaticRAM();

  inline auto data() -> uint8*;
  inline auto size() const -> uint;

  inline auto read(uint16 addr, uint8 data = 0) -> uint8;
  inline auto write(uint16 addr, uint8 data) -> void;
  inline auto operator[](uint16 addr) -> uint8&;
  inline auto operator[](uint16 addr) const -> const uint8&;

private:
  uint8* _data = nullptr;
  uint _size = 0;
};

struct MappedRAM : Memory {
  inline auto reset() -> void;
  inline auto allocate(uint) -> void;

  inline auto writeProtect(bool writeProtect) -> void;
  inline auto data() -> uint8*;
  inline auto size() const -> uint;

  inline auto read(uint16 addr, uint8 data = 0) -> uint8;
  inline auto write(uint16 addr, uint8 data) -> void;
  inline auto operator[](uint16 addr) const -> const uint8&;

  inline auto readDirect(uint addr, uint8 data = 0) -> uint8;
  inline auto writeDirect(uint addr, uint8 data) -> void;

private:
  uint8* _data = nullptr;
  uint _size = 0;
  bool _writeProtect = false;
};

struct Bus {
  alwaysinline static auto mirror(uint addr, uint size) -> uint;
  alwaysinline static auto reduce(uint addr, uint mask) -> uint;

  ~Bus();

  alwaysinline auto read(uint16 addr, uint8 data) -> uint8;
  alwaysinline auto write(uint16 addr, uint8 data) -> void;

  alwaysinline auto readCHR(uint16 addr, uint8 data) -> uint8;
  alwaysinline auto writeCHR(uint16 addr, uint8 data) -> void;

  auto reset() -> void;
  auto map(
    const function<uint8 (uint16, uint8)>& read,
    const function<void (uint16, uint8)>& write,
    const string& addr, uint size = 0, uint base = 0, uint mask = 0
  ) -> void;
  auto unmap(const string& addr) -> void;

private:
  uint8* lookup = nullptr;
  uint32* target = nullptr;

  function<auto (uint16, uint8) -> uint8> reader[256];
  function<auto (uint16, uint8) -> void> writer[256];
  uint16 counter[256];
};

extern Bus bus;
