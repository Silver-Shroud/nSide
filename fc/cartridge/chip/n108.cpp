struct N108 : Chip {
  N108(Board& board, Markup::Node& board_node) : Chip(board) {
    string type = board_node["chip/type"].text();

    if(type.match("*108*")) revision = Revision::N108;
    if(type.match("*109*")) revision = Revision::N109;
    if(type.match("*118*")) revision = Revision::N118;
    if(type.match("*119*")) revision = Revision::N119;
  }

  auto prg_addr(uint addr) const -> uint {
    switch((addr >> 13) & 3) {
    case 0: return (prg_bank[0] << 13) | (addr & 0x1fff);
    case 1: return (prg_bank[1] << 13) | (addr & 0x1fff);
    case 2: return (0x0e << 13) | (addr & 0x1fff);
    case 3: return (0x0f << 13) | (addr & 0x1fff);
    }
  }

  auto chr_addr(uint addr) const -> uint {
    if(addr <= 0x07ff) return (chr_bank[0] << 10) | (addr & 0x07ff);
    if(addr <= 0x0fff) return (chr_bank[1] << 10) | (addr & 0x07ff);
    if(addr <= 0x13ff) return (chr_bank[2] << 10) | (addr & 0x03ff);
    if(addr <= 0x17ff) return (chr_bank[3] << 10) | (addr & 0x03ff);
    if(addr <= 0x1bff) return (chr_bank[4] << 10) | (addr & 0x03ff);
    if(addr <= 0x1fff) return (chr_bank[5] << 10) | (addr & 0x03ff);
  }

  auto reg_write(uint addr, uint8 data) -> void {
    switch(addr & 0x8001) {
    case 0x8000:
      bank_select = data & 0x07;
      break;

    case 0x8001:
      switch(bank_select) {
      case 0: chr_bank[0] = data & 0x3e; break;
      case 1: chr_bank[1] = data & 0x3e; break;
      case 2: chr_bank[2] = data & 0x3f; break;
      case 3: chr_bank[3] = data & 0x3f; break;
      case 4: chr_bank[4] = data & 0x3f; break;
      case 5: chr_bank[5] = data & 0x3f; break;
      case 6: prg_bank[0] = data & 0x0f; break;
      case 7: prg_bank[1] = data & 0x0f; break;
      }
      break;
    }
  }

  auto power() -> void {
  }

  auto reset() -> void {
    bank_select = 0;
    prg_bank[0] = 0;
    prg_bank[1] = 0;
    chr_bank[0] = 0;
    chr_bank[1] = 0;
    chr_bank[2] = 0;
    chr_bank[3] = 0;
    chr_bank[4] = 0;
    chr_bank[5] = 0;
  }

  auto serialize(serializer& s) -> void {
    s.integer(bank_select);
    s.array(prg_bank);
    s.array(chr_bank);
  }

  enum class Revision : uint {
    N108,
    N109,
    N118,
    N119,
  } revision;

  uint3 bank_select;
  uint8 prg_bank[2];
  uint8 chr_bank[6];
};
