#include "mode7.cpp"

PPU::Background::Background(PPU& self, uint_t id) : self(self), id(id) {
  priority0_enable = true;
  priority1_enable = true;

  opt_valid_bit = (id == ID::BG1 ? 0x2000 : id == ID::BG2 ? 0x4000 : 0x0000);

  mosaic_table = new uint16_t*[16];
  for(uint_t m = 0; m < 16; m++) {
    mosaic_table[m] = new uint16_t[4096];
    for(uint_t x = 0; x < 4096; x++) {
      mosaic_table[m][x] = (x / (m + 1)) * (m + 1);
    }
  }
}

PPU::Background::~Background() {
  for(uint_t m = 0; m < 16; m++) delete[] mosaic_table[m];
  delete[] mosaic_table;
}

auto PPU::Background::get_tile(uint_t hoffset, uint_t voffset) -> uint_t {
  uint_t tile_x = (hoffset & mask_x) >> tile_width;
  uint_t tile_y = (voffset & mask_y) >> tile_height;

  uint_t tile_pos = ((tile_y & 0x1f) << 5) + (tile_x & 0x1f);
  if(tile_y & 0x20) tile_pos += scy;
  if(tile_x & 0x20) tile_pos += scx;

  const uint16_t tiledata_addr = regs.screen_addr + (tile_pos << 1);
  return (ppu.vram[tiledata_addr + 0] << 0) + (ppu.vram[tiledata_addr + 1] << 8);
}

auto PPU::Background::offset_per_tile(uint_t x, uint_t y, uint_t& hoffset, uint_t& voffset) -> void {
  uint_t opt_x = (x + (hscroll & 7)), hval, vval;
  if(opt_x >= 8) {
    hval = self.bg3.get_tile((opt_x - 8) + (self.bg3.regs.hoffset & ~7), self.bg3.regs.voffset + 0);
    if(self.regs.bgmode != 4)
    vval = self.bg3.get_tile((opt_x - 8) + (self.bg3.regs.hoffset & ~7), self.bg3.regs.voffset + 8);

    if(self.regs.bgmode == 4) {
      if(hval & opt_valid_bit) {
        if(!(hval & 0x8000)) {
          hoffset = opt_x + (hval & ~7);
        } else {
          voffset = y + hval;
        }
      }
    } else {
      if(hval & opt_valid_bit) {
        hoffset = opt_x + (hval & ~7);
      }
      if(vval & opt_valid_bit) {
        voffset = y + vval;
      }
    }
  }
}

auto PPU::Background::scanline() -> void {
  if(self.vcounter() == 1) {
    mosaic_vcounter = regs.mosaic + 1;
    mosaic_voffset = 1;
  } else if(--mosaic_vcounter == 0) {
    mosaic_vcounter = regs.mosaic + 1;
    mosaic_voffset += regs.mosaic + 1;
  }
  if(self.regs.display_disable) return;

  hires = (self.regs.bgmode == 5 || self.regs.bgmode == 6);
  width = !hires ? 256 : 512;

  tile_height = regs.tile_size ? 4 : 3;
  tile_width = hires ? 4 : tile_height;

  mask_x = (tile_height == 4 ? width << 1 : width);
  mask_y = mask_x;
  if(regs.screen_size & 1) mask_x <<= 1;
  if(regs.screen_size & 2) mask_y <<= 1;
  mask_x--;
  mask_y--;

  scx = (regs.screen_size & 1 ? 32 << 5 : 0);
  scy = (regs.screen_size & 2 ? 32 << 5 : 0);
  if(regs.screen_size == 3) scy <<= 1;
}

auto PPU::Background::render() -> void {
  if(regs.mode == Mode::Inactive) return;
  if(regs.main_enable == false && regs.sub_enable == false) return;

  if(regs.main_enable) window.render(0);
  if(regs.sub_enable) window.render(1);
  if(regs.mode == Mode::Mode7) return render_mode7();

  uint_t priority0 = (priority0_enable ? regs.priority0 : 0);
  uint_t priority1 = (priority1_enable ? regs.priority1 : 0);
  if(priority0 + priority1 == 0) return;

  uint_t mosaic_hcounter = 1;
  uint_t mosaic_palette = 0;
  uint_t mosaic_priority = 0;
  uint_t mosaic_color = 0;

  const uint_t bgpal_index = (self.regs.bgmode == 0 ? id << 5 : 0);
  const uint_t pal_size = 2 << regs.mode;
  const uint_t tile_mask = 0x0fff >> regs.mode;
  const uint_t tiledata_index = regs.tiledata_addr >> (4 + regs.mode);

  hscroll = regs.hoffset;
  vscroll = regs.voffset;

  uint_t y = (regs.mosaic == 0 ? self.vcounter() : mosaic_voffset);
  if(hires) {
    hscroll <<= 1;
    if(self.regs.interlace) y = (y << 1) + self.field();
  }

  uint_t tile_pri, tile_num;
  uint_t pal_index, pal_num;
  uint_t hoffset, voffset, col;
  bool mirror_x, mirror_y;

  const bool is_opt_mode = (self.regs.bgmode == 2 || self.regs.bgmode == 4 || self.regs.bgmode == 6);
  const bool is_direct_color_mode = (self.screen.regs.direct_color == true && id == ID::BG1 && (self.regs.bgmode == 3 || self.regs.bgmode == 4));

  int_t x = 0 - (hscroll & 7);
  while(x < width) {
    hoffset = x + hscroll;
    voffset = y + vscroll;
    if(is_opt_mode) offset_per_tile(x, y, hoffset, voffset);
    hoffset &= mask_x;
    voffset &= mask_y;

    tile_num = get_tile(hoffset, voffset);
    mirror_y = tile_num & 0x8000;
    mirror_x = tile_num & 0x4000;
    tile_pri = tile_num & 0x2000 ? priority1 : priority0;
    pal_num = (tile_num >> 10) & 7;
    pal_index = (bgpal_index + (pal_num << pal_size)) & 0xff;

    if(tile_width  == 4 && (bool)(hoffset & 8) != mirror_x) tile_num +=  1;
    if(tile_height == 4 && (bool)(voffset & 8) != mirror_y) tile_num += 16;
    tile_num = ((tile_num & 0x03ff) + tiledata_index) & tile_mask;

    if(mirror_y) voffset ^= 7;
    uint_t mirror_xmask = !mirror_x ? 0 : 7;

    uint8_t* tiledata = self.cache.tile(regs.mode, tile_num);
    tiledata += ((voffset & 7) * 8);

    for(uint_t n = 0; n < 8; n++, x++) {
      if(x & width) continue;
      if(--mosaic_hcounter == 0) {
        mosaic_hcounter = regs.mosaic + 1;
        mosaic_palette = tiledata[n ^ mirror_xmask];
        mosaic_priority = tile_pri;
        if(is_direct_color_mode) {
          mosaic_color = self.screen.get_direct_color(pal_num, mosaic_palette);
        } else {
          mosaic_color = self.screen.get_palette(pal_index + mosaic_palette);
        }
      }
      if(mosaic_palette == 0) continue;

      if(hires == false) {
        if(regs.main_enable && !window.main[x]) self.screen.output.plot_main(x, mosaic_color, mosaic_priority, id);
        if(regs.sub_enable && !window.sub[x]) self.screen.output.plot_sub(x, mosaic_color, mosaic_priority, id);
      } else {
        int_t half_x = x >> 1;
        if(x & 1) {
          if(regs.main_enable && !window.main[half_x]) self.screen.output.plot_main(half_x, mosaic_color, mosaic_priority, id);
        } else {
          if(regs.sub_enable && !window.sub[half_x]) self.screen.output.plot_sub(half_x, mosaic_color, mosaic_priority, id);
        }
      }
    }
  }
}
