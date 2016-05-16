struct PPU : Thread, PPUcounter {
  enum class Revision : uint {
    //YIQ
    RP2C02C,
    RP2C02G,

    //RGB
    RP2C03B,
    RP2C03G,
    RP2C04_0001,
    RP2C04_0002,
    RP2C04_0003,
    RP2C04_0004,
    RC2C03B,
    RC2C03C,
    RC2C05_01,
    RC2C05_02,
    RC2C05_03,
    RC2C05_04,
    RC2C05_05,

    //YUV
    RP2C07,
    UA6538,
  } revision;

  alwaysinline auto ntsc() const -> bool { return revision >= Revision::RP2C02C && revision <= Revision::RP2C02G; }
  alwaysinline auto pal()  const -> bool { return revision >= Revision::RP2C07  && revision <= Revision::UA6538;  }
  alwaysinline auto rgb()  const -> bool { return revision >= Revision::RP2C03B && revision <= Revision::RC2C05_05; }

  PPU();
  ~PPU();

  alwaysinline auto step(uint clocks) -> void;
  alwaysinline auto synchronizeCPU() -> void;

  static auto Enter() -> void;
  auto main() -> void;
  auto power() -> void;
  auto reset() -> void;

  auto origin_x() -> uint;
  auto origin_y() -> uint;

  auto raster_enable() const -> bool;
  auto nametable_addr() const -> uint;
  auto scrollx() const -> uint;
  auto scrolly() const -> uint;
  auto sprite_height() const -> uint;

  auto chr_load(uint14 addr) -> uint8;

  auto scrollx_increment() -> void;
  auto scrolly_increment() -> void;

  auto ext() -> uint4;

  auto raster_pixel() -> void;
  auto raster_sprite() -> void;
  auto raster_scanline() -> void;

  auto serialize(serializer&) -> void;

  //memory.cpp
  auto ciramRead(uint14 addr) -> uint8;
  auto ciramWrite(uint14 addr, uint8 data) -> void;
  alwaysinline auto cgramRead(uint14 addr) -> uint8;
  alwaysinline auto cgramWrite(uint14 addr, uint8 data) -> void;
  alwaysinline auto oamRead(uint8 addr) -> uint8;
  alwaysinline auto oamWrite(uint8 addr, uint8 data) -> void;

  //mmio.cpp
  auto read(uint16 addr, uint8 data) -> uint8;
  auto write(uint16 addr, uint8 data) -> void;

  uint8 ciram[4096]; // 2048 in Famicom and PlayChoice-10
  uint8 cgram[32];
  uint8 oam[256];

  bool side; // VS. System; 0: main, 1: sub

  static const uint9 RP2C03[16 * 4];
  static const uint9 RP2C04_0001[16 * 4];
  static const uint9 RP2C04_0002[16 * 4];
  static const uint9 RP2C04_0003[16 * 4];
  static const uint9 RP2C04_0004[16 * 4];

  //debugger functions
  auto exportRegisters(string &markup) -> void;

  struct Status {
    uint14 chr_abus;
    uint8 mdr;
    uint mdr_decay[8];

    uint8 bus_data;

    bool address_latch;

    uint15 vaddr;
    uint15 taddr;
    uint8 xaddr;

    bool nmi_hold;
    bool nmi_flag;

    //$2000
    bool nmi_enable;
    bool master_select;
    bool sprite_size;
    uint bg_addr;
    uint sprite_addr;
    uint vram_increment;

    //$2001
    uint3 emphasis;
    bool sprite_enable;
    bool bg_enable;
    bool sprite_edge_enable;
    bool bg_edge_enable;
    bool grayscale;

    //$2002
    bool sprite_zero_hit;
    bool sprite_overflow;

    //$2003
    uint8 oam_addr;
  } status;

  struct Raster {
    uint16 nametable;
    uint16 attribute;
    uint16 tiledatalo;
    uint16 tiledatahi;

    uint oam_iterator;
    uint oam_counter;

    struct OAM {
      uint8 id;
      uint8 y;
      uint8 tile;
      uint8 attr;
      uint8 x;

      uint8 tiledatalo;
      uint8 tiledatahi;
    } oam[8], soam[8];
  } raster;

privileged:
  uint32* output = nullptr;

  alwaysinline auto addClocks(uint) -> void;

  auto scanline() -> void;
  auto frame() -> void;
  auto refresh() -> void;

  friend class BeamGun;
  friend class Scheduler;
};

extern PPU ppu;
