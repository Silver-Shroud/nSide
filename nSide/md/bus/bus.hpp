struct BusCPU : Processor::M68K::Bus {
  auto load(Markup::Node) -> bool;

  auto readByte(uint24 addr) -> uint16 override;
  auto readByte_(uint24 addr) -> uint16;
  auto readWord(uint24 addr) -> uint16 override;
  auto readWord_(uint24 addr) -> uint16;
  auto writeByte(uint24 addr, uint16 data) -> void override;
  auto writeWord(uint24 addr, uint16 data) -> void override;

  auto readIO(uint24 addr) -> uint16;
  auto writeIO(uint24 addr, uint16 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  uint8 ram[64 * 1024];
};

struct BusAPU : Processor::Z80::Bus {
  auto read(uint16 addr) -> uint8 override;
  auto write(uint16 addr, uint8 data) -> void override;

  auto in(uint8 addr) -> uint8 override;
  auto out(uint8 addr, uint8 data) -> void override;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  uint8 ram[8 * 1024];
  uint9 bank;
};

extern BusCPU busCPU;
extern BusAPU busAPU;
