auto PPU::read(uint24 addr, uint8 data) -> uint8 {
  cpu.synchronizePPU();

  switch(addr & 0xffff) {
  case 0x2104: case 0x2105: case 0x2106: case 0x2108:
  case 0x2109: case 0x210a: case 0x2114: case 0x2115:
  case 0x2116: case 0x2118: case 0x2119: case 0x211a:
  case 0x2124: case 0x2125: case 0x2126: case 0x2128:
  case 0x2129: case 0x212a: {
    return regs.ppu1_mdr;
  }

  //MPYL
  case 0x2134: {
    uint32 r;
    r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
    regs.ppu1_mdr = r;
    return regs.ppu1_mdr;
  }

  //MPYM
  case 0x2135: {
    uint32 r;
    r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
    regs.ppu1_mdr = r >> 8;
    return regs.ppu1_mdr;
  }

  //MPYH
  case 0x2136: {
    uint32 r;
    r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
    regs.ppu1_mdr = r >> 16;
    return regs.ppu1_mdr;
  }

  //SLHV
  case 0x2137: {
    if(cpu.pio() & 0x80) {
      latchCounters();
    }
    return cpu.r.mdr;
  }

  //OAMDATAREAD
  case 0x2138: {
    regs.ppu1_mdr = oamRead(regs.oam_addr);

    regs.oam_addr++;
    regs.oam_addr &= 0x03ff;
    sprite.regs.first_sprite = !regs.oam_priority ? 0 : (regs.oam_addr >> 2) & 127;

    return regs.ppu1_mdr;
  }

  //VMDATALREAD
  case 0x2139: {
    uint16 addr = getVramAddress();
    regs.ppu1_mdr = regs.vram_readbuffer;
    if(regs.vram_incmode == 0) {
      addr &= 0xfffe;
      regs.vram_readbuffer  = vramRead(addr + 0);
      regs.vram_readbuffer |= vramRead(addr + 1) << 8;
      regs.vram_addr += regs.vram_incsize;
    }
    return regs.ppu1_mdr;
  }

  //VMDATAHREAD
  case 0x213a: {
    uint16 addr = getVramAddress() + 1;
    regs.ppu1_mdr = regs.vram_readbuffer >> 8;
    if(regs.vram_incmode == 1) {
      addr &= 0xfffe;
      regs.vram_readbuffer  = vramRead(addr + 0);
      regs.vram_readbuffer |= vramRead(addr + 1) << 8;
      regs.vram_addr += regs.vram_incsize;
    }
    return regs.ppu1_mdr;
  }

  //CGDATAREAD
  //note: CGRAM palette data is 15-bits (0,bbbbb,ggggg,rrrrr)
  //therefore, the high byte read from each color does not
  //update bit 7 of the PPU2 MDR.
  case 0x213b: {
    if(!(regs.cgram_addr & 1)) {
      regs.ppu2_mdr  = cgramRead(regs.cgram_addr) & 0xff;
    } else {
      regs.ppu2_mdr &= 0x80;
      regs.ppu2_mdr |= cgramRead(regs.cgram_addr) & 0x7f;
    }
    regs.cgram_addr++;
    regs.cgram_addr &= 0x01ff;
    return regs.ppu2_mdr;
  }

  //OPHCT
  case 0x213c: {
    if(!regs.latch_hcounter) {
      regs.ppu2_mdr  = regs.hcounter & 0xff;
    } else {
      regs.ppu2_mdr &= 0xfe;
      regs.ppu2_mdr |= (regs.hcounter >> 8) & 1;
    }
    regs.latch_hcounter ^= 1;
    return regs.ppu2_mdr;
  }

  //OPVCT
  case 0x213d: {
    if(regs.latch_vcounter == 0) {
      regs.ppu2_mdr  = regs.vcounter & 0xff;
    } else {
      regs.ppu2_mdr &= 0xfe;
      regs.ppu2_mdr |= (regs.vcounter >> 8) & 1;
    }
    regs.latch_vcounter ^= 1;
    return regs.ppu2_mdr;
  };

  //STAT77
  case 0x213e: {
    uint8 r = 0x00;
    r |= sprite.regs.time_over  ? 0x80 : 0x00;
    r |= sprite.regs.range_over ? 0x40 : 0x00;
    r |= regs.ppu1_mdr & 0x10;
    r |= ppu1_version & 0x0f;
    regs.ppu1_mdr = r;
    return regs.ppu1_mdr;
  }

  //STAT78
  case 0x213f: {
    uint8 r = 0x00;
    regs.latch_hcounter = 0;
    regs.latch_vcounter = 0;

    r |= cpu.field() << 7;
    if((cpu.pio() & 0x80) == 0) {
      r |= 0x40;
    } else if(regs.counters_latched) {
      r |= 0x40;
      regs.counters_latched = false;
    }
    r |= (regs.ppu2_mdr & 0x20);
    r |= (system.region() == System::Region::NTSC ? 0 : 1) << 4;
    r |= ppu2_version & 0x0f;
    regs.ppu2_mdr = r;
    return regs.ppu2_mdr;
  }

  }

  return data;
}

auto PPU::write(uint24 addr, uint8 data) -> void {
  cpu.synchronizePPU();

  switch(addr & 0xffff) {

  //INIDISP
  case 0x2100: {
    if(regs.display_disable && cpu.vcounter() == vdisp()) {
      regs.oam_addr = regs.oam_baseaddr << 1;
      sprite.regs.first_sprite = !regs.oam_priority ? 0 : (regs.oam_addr >> 2) & 127;
    }

    regs.display_disable = data & 0x80;
    regs.display_brightness = data & 0x0f;
    return;
  }

  //OBSEL
  case 0x2101: {
    sprite.regs.base_size = (data >> 5) & 7;
    sprite.regs.nameselect = (data >> 3) & 3;
    sprite.regs.tiledata_addr = (data & 3) << 14;
    return;
  }

  //OAMADDL
  case 0x2102: {
    regs.oam_baseaddr        = (regs.oam_baseaddr & ~0xff) | (data << 0);
    regs.oam_baseaddr       &= 0x01ff;
    regs.oam_addr            = regs.oam_baseaddr << 1;
    sprite.regs.first_sprite = !regs.oam_priority ? 0 : (regs.oam_addr >> 2) & 127;
    return;
  }

  //OAMADDH
  case 0x2103: {
    regs.oam_priority        = data & 0x80;
    regs.oam_baseaddr        = (regs.oam_baseaddr &  0xff) | (data << 8);
    regs.oam_baseaddr       &= 0x01ff;
    regs.oam_addr            = regs.oam_baseaddr << 1;
    sprite.regs.first_sprite = !regs.oam_priority ? 0 : (regs.oam_addr >> 2) & 127;
    return;
  }

  //OAMDATA
  case 0x2104: {
    if((regs.oam_addr & 1) == 0) regs.oam_latchdata = data;

    if(regs.oam_addr & 0x0200) {
      oamWrite(regs.oam_addr, data);
    } else if((regs.oam_addr & 1) == 1) {
      oamWrite((regs.oam_addr & ~1) + 0, regs.oam_latchdata);
      oamWrite((regs.oam_addr & ~1) + 1, data);
    }

    regs.oam_addr++;
    regs.oam_addr &= 0x03ff;
    sprite.regs.first_sprite = !regs.oam_priority ? 0 : (regs.oam_addr >> 2) & 127;
    return;
  }

  //BGMODE
  case 0x2105: {
    bg4.regs.tile_size = (data & 0x80);
    bg3.regs.tile_size = (data & 0x40);
    bg2.regs.tile_size = (data & 0x20);
    bg1.regs.tile_size = (data & 0x10);
    regs.bg3_priority = (data & 0x08);
    regs.bgmode = (data & 0x07);
    return;
  }

  //MOSAIC
  case 0x2106: {
    regs.mosaic_size = (data >> 4) & 15;
    bg4.regs.mosaic_enabled = data & 0x08;
    bg3.regs.mosaic_enabled = data & 0x04;
    bg2.regs.mosaic_enabled = data & 0x02;
    bg1.regs.mosaic_enabled = data & 0x01;
    return;
  }

  //BG1SC
  case 0x2107: {
    bg1.regs.screen_addr = (data & 0x7c) << 9;
    bg1.regs.screen_size = data & 3;
    return;
  }

  //BG2SC
  case 0x2108: {
    bg2.regs.screen_addr = (data & 0x7c) << 9;
    bg2.regs.screen_size = data & 3;
    return;
  }

  //BG3SC
  case 0x2109: {
    bg3.regs.screen_addr = (data & 0x7c) << 9;
    bg3.regs.screen_size = data & 3;
    return;
  }

  //BG4SC
  case 0x210a: {
    bg4.regs.screen_addr = (data & 0x7c) << 9;
    bg4.regs.screen_size = data & 3;
    return;
  }

  //BG12NBA
  case 0x210b: {
    bg1.regs.tiledata_addr = (data & 0x07) << 13;
    bg2.regs.tiledata_addr = (data & 0x70) <<  9;
    return;
  }

  //BG34NBA
  case 0x210c: {
    bg3.regs.tiledata_addr = (data & 0x07) << 13;
    bg4.regs.tiledata_addr = (data & 0x70) <<  9;
    return;
  }

  //BG1HOFS
  case 0x210d: {
    regs.mode7_hoffset = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;

    bg1.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg1.regs.hoffset >> 8) & 7);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG1VOFS
  case 0x210e: {
    regs.mode7_voffset = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;

    bg1.regs.voffset = (data << 8) | (regs.bgofs_latchdata);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG2HOFS
  case 0x210f: {
    bg2.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg2.regs.hoffset >> 8) & 7);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG2VOFS
  case 0x2110: {
    bg2.regs.voffset = (data << 8) | (regs.bgofs_latchdata);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG3HOFS
  case 0x2111: {
    bg3.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg3.regs.hoffset >> 8) & 7);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG3VOFS
  case 0x2112: {
    bg3.regs.voffset = (data << 8) | (regs.bgofs_latchdata);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG4HOFS
  case 0x2113: {
    bg4.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg4.regs.hoffset >> 8) & 7);
    regs.bgofs_latchdata = data;
    return;
  }

  //BG4VOFS
  case 0x2114: {
    bg4.regs.voffset = (data << 8) | (regs.bgofs_latchdata);
    regs.bgofs_latchdata = data;
    return;
  }

  //VMAIN
  case 0x2115: {
    regs.vram_incmode = !!(data & 0x80);
    regs.vram_mapping = (data >> 2) & 3;
    switch(data & 3) {
    case 0: regs.vram_incsize =   1; break;
    case 1: regs.vram_incsize =  32; break;
    case 2: regs.vram_incsize = 128; break;
    case 3: regs.vram_incsize = 128; break;
    }
    return;
  }

  //VMADDL
  case 0x2116: {
    regs.vram_addr = (regs.vram_addr & 0xff00) | data;
    uint16 addr = getVramAddress();
    regs.vram_readbuffer  = vramRead(addr + 0);
    regs.vram_readbuffer |= vramRead(addr + 1) << 8;
    return;
  }

  //VMADDH
  case 0x2117: {
    regs.vram_addr = (data << 8) | (regs.vram_addr & 0x00ff);
    uint16 addr = getVramAddress();
    regs.vram_readbuffer  = vramRead(addr + 0);
    regs.vram_readbuffer |= vramRead(addr + 1) << 8;
    return;
  }

  //VMDATAL
  case 0x2118: {
    uint16 addr = getVramAddress();
    vramWrite(addr, data);
    bg_tiledata_state[Background::Mode::BPP2][addr >> 4] = 1;
    bg_tiledata_state[Background::Mode::BPP4][addr >> 5] = 1;
    bg_tiledata_state[Background::Mode::BPP8][addr >> 6] = 1;

    if(regs.vram_incmode == 0) {
      regs.vram_addr += regs.vram_incsize;
    }
    return;
  }

  //VMDATAH
  case 0x2119: {
    uint16 addr = getVramAddress() + 1;
    vramWrite(addr, data);
    bg_tiledata_state[Background::Mode::BPP2][addr >> 4] = 1;
    bg_tiledata_state[Background::Mode::BPP4][addr >> 5] = 1;
    bg_tiledata_state[Background::Mode::BPP8][addr >> 6] = 1;

    if(regs.vram_incmode == 1) {
      regs.vram_addr += regs.vram_incsize;
    }
    return;
  }

  //M7SEL
  case 0x211a: {
    regs.mode7_repeat = (data >> 6) & 3;
    regs.mode7_vflip = data & 0x02;
    regs.mode7_hflip = data & 0x01;
    return;
  }

  //M7A
  case 0x211b: {
    regs.m7a = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //M7B
  case 0x211c: {
    regs.m7b = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //M7C
  case 0x211d: {
    regs.m7c = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //M7D
  case 0x211e: {
    regs.m7d = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //M7X
  case 0x211f: {
    regs.m7x = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //M7Y
  case 0x2120: {
    regs.m7y = (data << 8) | regs.mode7_latchdata;
    regs.mode7_latchdata = data;
    return;
  }

  //CGADD
  case 0x2121: {
    regs.cgram_addr = data << 1;
    return;
  }

  //CGDATA
  //note: CGRAM palette data format is 15-bits
  //(0,bbbbb,ggggg,rrrrr). Highest bit is ignored,
  //as evidenced by $213b CGRAM data reads.
  //
  //anomie indicates writes to CGDATA work the same
  //as writes to OAMDATA's low table. need to verify
  //this on hardware.
  case 0x2122: {
    if(!(regs.cgram_addr & 1)) {
      regs.cgram_latchdata = data;
    } else {
      cgramWrite((regs.cgram_addr & 0x01fe),     regs.cgram_latchdata);
      cgramWrite((regs.cgram_addr & 0x01fe) + 1, data & 0x7f);
    }
    regs.cgram_addr++;
    regs.cgram_addr &= 0x01ff;
    return;
  }

  //W12SEL
  case 0x2123: {
    window.regs.bg2_two_enable = data & 0x80;
    window.regs.bg2_two_invert = data & 0x40;
    window.regs.bg2_one_enable = data & 0x20;
    window.regs.bg2_one_invert = data & 0x10;
    window.regs.bg1_two_enable = data & 0x08;
    window.regs.bg1_two_invert = data & 0x04;
    window.regs.bg1_one_enable = data & 0x02;
    window.regs.bg1_one_invert = data & 0x01;
    return;
  }

  //W34SEL
  case 0x2124: {
    window.regs.bg4_two_enable = data & 0x80;
    window.regs.bg4_two_invert = data & 0x40;
    window.regs.bg4_one_enable = data & 0x20;
    window.regs.bg4_one_invert = data & 0x10;
    window.regs.bg3_two_enable = data & 0x08;
    window.regs.bg3_two_invert = data & 0x04;
    window.regs.bg3_one_enable = data & 0x02;
    window.regs.bg3_one_invert = data & 0x01;
    return;
  }

  //WOBJSEL
  case 0x2125: {
    window.regs.col_two_enable = data & 0x80;
    window.regs.col_two_invert = data & 0x40;
    window.regs.col_one_enable = data & 0x20;
    window.regs.col_one_invert = data & 0x10;
    window.regs.oam_two_enable = data & 0x08;
    window.regs.oam_two_invert = data & 0x04;
    window.regs.oam_one_enable = data & 0x02;
    window.regs.oam_one_invert = data & 0x01;
    return;
  }

  //WH0
  case 0x2126: {
    window.regs.one_left = data;
    return;
  }

  //WH1
  case 0x2127: {
    window.regs.one_right = data;
    return;
  }

  //WH2
  case 0x2128: {
    window.regs.two_left = data;
    return;
  }

  //WH3
  case 0x2129: {
    window.regs.two_right = data;
    return;
  }

  //WBGLOG
  case 0x212a: {
    window.regs.bg4_mask = (data >> 6) & 3;
    window.regs.bg3_mask = (data >> 4) & 3;
    window.regs.bg2_mask = (data >> 2) & 3;
    window.regs.bg1_mask = (data     ) & 3;
    return;
  }

  //WOBJLOG
  case 0x212b: {
    window.regs.col_mask = (data >> 2) & 3;
    window.regs.oam_mask = (data     ) & 3;
    return;
  }

  //TM
  case 0x212c: {
    sprite.regs.main_enable = data & 0x10;
    bg4.regs.main_enable = data & 0x08;
    bg3.regs.main_enable = data & 0x04;
    bg2.regs.main_enable = data & 0x02;
    bg1.regs.main_enable = data & 0x01;
    return;
  }

  //TS
  case 0x212d: {
    sprite.regs.sub_enable = data & 0x10;
    bg4.regs.sub_enable = data & 0x08;
    bg3.regs.sub_enable = data & 0x04;
    bg2.regs.sub_enable = data & 0x02;
    bg1.regs.sub_enable = data & 0x01;
    return;
  }

  //TMW
  case 0x212e: {
    window.regs.oam_main_enable = data & 0x10;
    window.regs.bg4_main_enable = data & 0x08;
    window.regs.bg3_main_enable = data & 0x04;
    window.regs.bg2_main_enable = data & 0x02;
    window.regs.bg1_main_enable = data & 0x01;
    return;
  }

  //TSW
  case 0x212f: {
    window.regs.oam_sub_enable = data & 0x10;
    window.regs.bg4_sub_enable = data & 0x08;
    window.regs.bg3_sub_enable = data & 0x04;
    window.regs.bg2_sub_enable = data & 0x02;
    window.regs.bg1_sub_enable = data & 0x01;
    return;
  }

  //CGWSEL
  case 0x2130: {
    window.regs.col_main_mask = (data >> 6) & 3;
    window.regs.col_sub_mask = (data >> 4) & 3;
    screen.regs.addsub_mode = data & 0x02;
    screen.regs.direct_color = data & 0x01;
    return;
  }

  //CGADDSUB
  case 0x2131: {
    screen.regs.color_mode = data & 0x80;
    screen.regs.color_halve = data & 0x40;
    screen.regs.back_color_enable = data & 0x20;
    screen.regs.oam_color_enable = data & 0x10;
    screen.regs.bg4_color_enable = data & 0x08;
    screen.regs.bg3_color_enable = data & 0x04;
    screen.regs.bg2_color_enable = data & 0x02;
    screen.regs.bg1_color_enable = data & 0x01;
    return;
  }

  //COLDATA
  case 0x2132: {
    if(data & 0x80) screen.regs.color_b = data & 0x1f;
    if(data & 0x40) screen.regs.color_g = data & 0x1f;
    if(data & 0x20) screen.regs.color_r = data & 0x1f;

    regs.color_rgb = (screen.regs.color_r      )
                   | (screen.regs.color_g <<  5)
                   | (screen.regs.color_b << 10);
    return;
  }

  //SETINI
  case 0x2133: {
    regs.mode7_extbg = data & 0x40;
    regs.pseudo_hires = data & 0x08;
    regs.overscan = data & 0x04;
    sprite.regs.interlace = data & 0x02;
    regs.interlace = data & 0x01;

    display.overscan = regs.overscan;
    sprite_list_valid = false;
    return;
  }

  }
}

auto PPU::latchCounters() -> void {
  regs.hcounter = cpu.hdot();
  regs.vcounter = cpu.vcounter();
  regs.counters_latched = true;
}