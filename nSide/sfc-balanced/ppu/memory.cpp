auto PPU::getVramAddress() -> uint16 {
  uint16 addr = regs.vram_addr;
  switch(regs.vram_mapping) {
    case 0: break;  //direct mapping
    case 1: addr = (addr & 0xff00) | ((addr & 0x001f) << 3) | ((addr >> 5) & 7); break;
    case 2: addr = (addr & 0xfe00) | ((addr & 0x003f) << 3) | ((addr >> 6) & 7); break;
    case 3: addr = (addr & 0xfc00) | ((addr & 0x007f) << 3) | ((addr >> 7) & 7); break;
  }
  return (addr << 1);
}

//NOTE: all VRAM writes during active display are invalid. Unlike OAM and CGRAM, they will
//not be written anywhere at all. The below address ranges for where writes are invalid have
//been validated on hardware, as has the edge case where the S-CPU MDR can be written if the
//write occurs during the very last clock cycle of vblank.

auto PPU::vramRead(uint16 addr) -> uint8 {
  uint8 data;

  if(regs.display_disable) {
    data = vram[addr];
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    uint16 ls = ((system.region() == System::Region::NTSC ? 525 : 625) >> 1) - 1;
    if(interlace() && !cpu.field()) ls++;

    if(v == ls && h == 1362) {
      data = 0x00;
    } else if(v < vdisp() - 1) {
      data = 0x00;
    } else if(v == vdisp() - 1) {
      if(h == 1362) {
        data = vram[addr];
      } else {
        data = 0x00;
      }
    } else {
      data = vram[addr];
    }
  }

  return data;
}

auto PPU::vramWrite(uint16 addr, uint8 data) -> void {
  if(regs.display_disable) {
    vram[addr] = data;
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v == 0) {
      if(h <= 4) {
        vram[addr] = data;
      } else if(h == 6) {
        vram[addr] = cpu.r.mdr;
      } else {
        //no write
      }
    } else if(v < vdisp()) {
      //no write
    } else if(v == vdisp()) {
      if(h <= 4) {
        //no write
      } else {
        vram[addr] = data;
      }
    } else {
      vram[addr] = data;
    }
  }
}

auto PPU::oamRead(uint16 addr) -> uint8 {
  addr &= 0x03ff;
  if(addr & 0x0200) addr &= 0x021f;
  uint8 data;

  if(regs.display_disable) {
    data = oam[addr];
  } else {
    if(cpu.vcounter() < vdisp()) {
      data = oam[regs.oam_iaddr];
    } else {
      data = oam[addr];
    }
  }

  return data;
}

auto PPU::oamWrite(uint16 addr, uint8 data) -> void {
  addr &= 0x03ff;
  if(addr & 0x0200) addr &= 0x021f;

  sprite_list_valid = false;

  if(regs.display_disable) {
    oam[addr] = data;
    update_sprite_list(addr, data);
  } else {
    if(cpu.vcounter() < vdisp()) {
      oam[regs.oam_iaddr] = data;
      update_sprite_list(regs.oam_iaddr, data);
    } else {
      oam[addr] = data;
      update_sprite_list(addr, data);
    }
  }
}

auto PPU::cgramRead(uint16 addr) -> uint8 {
  addr &= 0x01ff;
  uint8 data;

  if(1 || regs.display_disable) {
    data = cgram[addr];
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v < vdisp() && h >= 128 && h < 1096) {
      data = cgram[regs.cgram_iaddr] & 0x7f;
    } else {
      data = cgram[addr];
    }
  }

  if(addr & 1) data &= 0x7f;
  return data;
}

auto PPU::cgramWrite(uint16 addr, uint8 data) -> void {
  addr &= 0x01ff;
  if(addr & 1) data &= 0x7f;

  if(1 || regs.display_disable) {
    cgram[addr] = data;
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v < vdisp() && h >= 128 && h < 1096) {
      cgram[regs.cgram_iaddr] = data & 0x7f;
    } else {
      cgram[addr] = data;
    }
  }
}