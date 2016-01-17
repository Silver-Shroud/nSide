//HVC-FAMILYBASIC
//HVC-HROM
//HVC-NROM-128
//HVC-NROM-256
//HVC-RROM
//HVC-RROM-128
//HVC-RTROM
//HVC-SROM
//HVC-STROM

struct HVC_NROM : Board {
  HVC_NROM(Markup::Node& board_node) : Board(board_node) {
    string type = board_node["id"].text();
    if(type.match("*FAMILYBASIC*")) revision = Revision::FAMILYBASIC;
    if(type.match("*HROM*"       )) revision = Revision::HROM;
    if(type.match("*RROM*"       )) revision = Revision::RROM;
    if(type.match("*SROM*"       )) revision = Revision::SROM;
    if(revision == Revision::HROM)
      settings.mirror = 0;
    else
      settings.mirror = board_node["mirror/mode"].text() == "horizontal";
  }

  auto prg_read(uint addr) -> uint8 {
    if((addr & 0x8000) == 0x8000) return read(prgrom, addr);
    if(revision == Revision::FAMILYBASIC && (addr & 0xe000) == 0x6000)
      return read(prgram, addr);
    return cpu.mdr();
  }

  auto prg_write(uint addr, uint8 data) -> void {
    if(revision == Revision::FAMILYBASIC && (addr & 0xe000) == 0x6000)
      write(prgram, addr, data);
  }

  auto chr_read(uint addr) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.ciram_read(addr & 0x07ff);
    }
    return Board::chr_read(addr);
  }

  auto chr_write(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.ciram_write(addr, data);
    }
    return Board::chr_write(addr, data);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
  }

  enum class Revision : uint {
    FAMILYBASIC,
    HROM,
    NROM,
    RROM,
    RTROM,
    SROM,
    STROM,
  } revision;

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
  } settings;
};