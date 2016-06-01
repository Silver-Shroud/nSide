auto PPU::update_sprite_list(uint addr, uint8 data) -> void {
  if(addr < 0x0200) {
    uint i = addr >> 2;
    switch(addr & 3) {
    case 0: sprite_list[i].x = (sprite_list[i].x & 0x0100) | data; break;
    case 1: sprite_list[i].y = (data + 1) & 0xff; break;
    case 2: sprite_list[i].character = data; break;
    case 3: sprite_list[i].vflip = data & 0x80;
            sprite_list[i].hflip = data & 0x40;
            sprite_list[i].priority = (data >> 4) & 3;
            sprite_list[i].palette = (data >> 1) & 7;
            sprite_list[i].use_nameselect = data & 0x01;
    }
  } else {
    uint i = (addr & 0x1f) << 2;
    sprite_list[i + 0].x = ((data & 0x01) << 8) | (sprite_list[i + 0].x & 0xff);
    sprite_list[i + 0].size = data & 0x02;
    sprite_list[i + 1].x = ((data & 0x04) << 6) | (sprite_list[i + 1].x & 0xff);
    sprite_list[i + 1].size = data & 0x08;
    sprite_list[i + 2].x = ((data & 0x10) << 4) | (sprite_list[i + 2].x & 0xff);
    sprite_list[i + 2].size = data & 0x20;
    sprite_list[i + 3].x = ((data & 0x40) << 2) | (sprite_list[i + 3].x & 0xff);
    sprite_list[i + 3].size = data & 0x80;
  }
}

auto PPU::build_sprite_list() -> void {
  if(sprite_list_valid) return;
  sprite_list_valid = true;

  for(uint i : range(128)) {
    const bool size = sprite_list[i].size;

    switch(cache.oam_basesize) {
    case 0: sprite_list[i].width  = (!size) ?  8 : 16;
            sprite_list[i].height = (!size) ?  8 : 16;
            break;
    case 1: sprite_list[i].width  = (!size) ?  8 : 32;
            sprite_list[i].height = (!size) ?  8 : 32;
            break;
    case 2: sprite_list[i].width  = (!size) ?  8 : 64;
            sprite_list[i].height = (!size) ?  8 : 64;
            break;
    case 3: sprite_list[i].width  = (!size) ? 16 : 32;
            sprite_list[i].height = (!size) ? 16 : 32;
            break;
    case 4: sprite_list[i].width  = (!size) ? 16 : 64;
            sprite_list[i].height = (!size) ? 16 : 64;
            break;
    case 5: sprite_list[i].width  = (!size) ? 32 : 64;
            sprite_list[i].height = (!size) ? 32 : 64;
            break;
    case 6: sprite_list[i].width  = (!size) ? 16 : 32;
            sprite_list[i].height = (!size) ? 32 : 64;
            if(sprite.regs.interlace && !size) sprite_list[i].height = 16;
            //32x64 height is not affected by sprite.regs.interlace setting
            break;
    case 7: sprite_list[i].width  = (!size) ? 16 : 32;
            sprite_list[i].height = (!size) ? 32 : 32;
            if(sprite.regs.interlace && !size) sprite_list[i].height = 16;
            break;
    }
  }
}

auto PPU::is_sprite_on_scanline() -> bool {
  //if sprite is entirely offscreen and doesn't wrap around to the left side of the screen,
  //then it is not counted. this *should* be 256, and not 255, even though dot 256 is offscreen.
  sprite_item* spr = &sprite_list[active_sprite];
  if(spr->x > 256 && (spr->x + spr->width - 1) < 512) return false;

  int spr_height = !sprite.regs.interlace ? (uint)spr->height : spr->height >> 1;
  if(line >= spr->y && line < (spr->y + spr_height)) return true;
  if((spr->y + spr_height) >= 256 && line < ((spr->y + spr_height) & 255)) return true;
  return false;
}

auto PPU::load_oam_tiles() -> void {
  sprite_item* spr = &sprite_list[active_sprite];
  uint16 tile_width = spr->width >> 3;
  int x = spr->x;
  int y = (line - spr->y) & 0xff;
  if(sprite.regs.interlace) {
    y <<= 1;
  }

  if(spr->vflip) {
    if(spr->width == spr->height) {
      y = (spr->height - 1) - y;
    } else {
      y = y < spr->width ? (spr->width - 1) - y : spr->width + ((spr->width - 1) - (y - spr->width));
    }
  }

  if(sprite.regs.interlace) {
    y = !spr->vflip ? y + field() : y - field();
  }

  x &= 511;
  y &= 255;

  uint16 tdaddr = cache.oam_tdaddr;
  uint16 chrx   = (spr->character     ) & 15;
  uint16 chry   = (spr->character >> 4) & 15;
  if(spr->use_nameselect) {
    tdaddr += (256 * 32) + (cache.oam_nameselect << 13);
  }
  chry  += (y >> 3);
  chry  &= 15;
  chry <<= 4;

  for(uint tx : range(tile_width)) {
    uint sx = (x + (tx << 3)) & 511;
    //ignore sprites that are offscreen, x==256 is a special case that loads all tiles in OBJ
    if(x != 256 && sx >= 256 && (sx + 7) < 512) continue;

    if(sprite.t.tile_count++ >= 34) break;
    uint n = sprite.t.tile_count - 1;
    oam_tilelist[n].x     = sx;
    oam_tilelist[n].y     = y;
    oam_tilelist[n].pri   = spr->priority;
    oam_tilelist[n].pal   = 128 + (spr->palette << 4);
    oam_tilelist[n].hflip = spr->hflip;

    uint mx  = !spr->hflip ? tx : (tile_width - 1) - tx;
    uint pos = tdaddr + ((chry + ((chrx + mx) & 15)) << 5);
    oam_tilelist[n].tile = (pos >> 5) & 0x07ff;
  }
}

auto PPU::render_oam_tile(int tile_num) -> void {
  oam_tileitem* t     = &oam_tilelist[tile_num];
  uint8* oam_td       = (uint8*)bg_tiledata[Background::Mode::BPP4];
  uint8* oam_td_state = (uint8*)bg_tiledata_state[Background::Mode::BPP4];

  if(oam_td_state[t->tile] == 1) {
    render_bg_tile<Background::Mode::BPP4>(t->tile);
  }

  uint sx = t->x;
  uint8* tile_ptr = (uint8*)oam_td + (t->tile << 6) + ((t->y & 7) << 3);
  for(uint x : range(8)) {
    sx &= 511;
    if(sx < 256) {
      uint col = *(tile_ptr + (!t->hflip ? x : 7 - x));
      if(col) {
        col += t->pal;
        oam_line_pal[sx] = col;
        oam_line_pri[sx] = t->pri;
      }
    }
    sx++;
  }
}

auto PPU::render_line_oam_rto() -> void {
  build_sprite_list();

  sprite.t.item_count = 0;
  sprite.t.tile_count = 0;

  memset(oam_line_pri, OAM_PRI_NONE, 256);
  memset(oam_itemlist, 0xff, 32);
  for(int s : range(34)) oam_tilelist[s].tile = 0xffff;

  for(int s : range(128)) {
    active_sprite = (s + sprite.regs.first_sprite) & 127;
    if(!is_sprite_on_scanline()) continue;
    if(sprite.t.item_count++ >= 32) break;
    oam_itemlist[sprite.t.item_count - 1] = (s + sprite.regs.first_sprite) & 127;
  }

  if(sprite.t.item_count > 0 && oam_itemlist[sprite.t.item_count - 1] != 0xff) {
    regs.oam_iaddr = 0x0200 + (oam_itemlist[sprite.t.item_count - 1] >> 2);
  }

  for(int s = 31; s >= 0; s--) {
    if(oam_itemlist[s] == 0xff) continue;
    active_sprite = oam_itemlist[s];
    load_oam_tiles();
  }

  sprite.regs.time_over  |= (sprite.t.tile_count > 34);
  sprite.regs.range_over |= (sprite.t.item_count > 32);
}

auto PPU::render_line_oam(uint8 pri0_pos, uint8 pri1_pos, uint8 pri2_pos, uint8 pri3_pos) -> void {
  if(pri0_pos + pri1_pos + pri2_pos + pri3_pos == 0) return;

  if(!sprite.regs.main_enable && !sprite.regs.sub_enable) return;

  for(uint s : range(34)) {
    if(oam_tilelist[s].tile == 0xffff) continue;
    render_oam_tile(s);
  }

  build_window_tables(OAM);
  uint8* wt_main = windowCache[OAM].main;
  uint8* wt_sub  = windowCache[OAM].sub;

  uint pri_tbl[4] = { pri0_pos, pri1_pos, pri2_pos, pri3_pos };
  #define setpixel_main(x) \
    if(pixelCache[x].pri_main < pri) { \
      pixelCache[x].pri_main = pri; \
      pixelCache[x].bg_main  = OAM; \
      pixelCache[x].src_main = get_palette(oam_line_pal[x]); \
      pixelCache[x].ce_main  = (oam_line_pal[x] < 192); \
    }
  #define setpixel_sub(x) \
    if(pixelCache[x].pri_sub < pri) { \
      pixelCache[x].pri_sub = pri; \
      pixelCache[x].bg_sub  = OAM; \
      pixelCache[x].src_sub = get_palette(oam_line_pal[x]); \
      pixelCache[x].ce_sub  = (oam_line_pal[x] < 192); \
    }
  for(int x : range(256)) {
    if(oam_line_pri[x] == OAM_PRI_NONE) continue;

    uint pri = pri_tbl[oam_line_pri[x]];
    if(sprite.regs.main_enable && !wt_main[x]) { setpixel_main(x); }
    if(sprite.regs.sub_enable  && !wt_sub[x])  { setpixel_sub(x); }
  }
  #undef setpixel_main
  #undef setpixel_sub
}
