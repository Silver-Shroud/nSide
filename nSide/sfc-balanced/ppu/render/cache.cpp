auto PPU::renderBGTile(uint color_depth, uint16 tile_num) -> void {
  uint8 col, d0, d1, d2, d3, d4, d5, d6, d7;

  uint8* dest = (uint8*)tiledataCache.tiledata[color_depth] + tile_num * 64;
  uint y = 8;

  #define renderBGTile_line_2bpp(mask) \
    col  = !!(d0 & mask) << 0; \
    col += !!(d1 & mask) << 1; \
    *dest++ = col

  #define renderBGTile_line_4bpp(mask) \
    col  = !!(d0 & mask) << 0; \
    col += !!(d1 & mask) << 1; \
    col += !!(d2 & mask) << 2; \
    col += !!(d3 & mask) << 3; \
    *dest++ = col

  #define renderBGTile_line_8bpp(mask) \
    col  = !!(d0 & mask) << 0; \
    col += !!(d1 & mask) << 1; \
    col += !!(d2 & mask) << 2; \
    col += !!(d3 & mask) << 3; \
    col += !!(d4 & mask) << 4; \
    col += !!(d5 & mask) << 5; \
    col += !!(d6 & mask) << 6; \
    col += !!(d7 & mask) << 7; \
    *dest++ = col

  if(color_depth == Background::Mode::BPP2) {
    uint pos = tile_num * 16;
    while(y--) {
      d0 = vram[pos +  0];
      d1 = vram[pos +  1];
      renderBGTile_line_2bpp(0x80);
      renderBGTile_line_2bpp(0x40);
      renderBGTile_line_2bpp(0x20);
      renderBGTile_line_2bpp(0x10);
      renderBGTile_line_2bpp(0x08);
      renderBGTile_line_2bpp(0x04);
      renderBGTile_line_2bpp(0x02);
      renderBGTile_line_2bpp(0x01);
      pos += 2;
    }
  }

  if(color_depth == Background::Mode::BPP4) {
    uint pos = tile_num * 32;
    while(y--) {
      d0 = vram[pos +  0];
      d1 = vram[pos +  1];
      d2 = vram[pos + 16];
      d3 = vram[pos + 17];
      renderBGTile_line_4bpp(0x80);
      renderBGTile_line_4bpp(0x40);
      renderBGTile_line_4bpp(0x20);
      renderBGTile_line_4bpp(0x10);
      renderBGTile_line_4bpp(0x08);
      renderBGTile_line_4bpp(0x04);
      renderBGTile_line_4bpp(0x02);
      renderBGTile_line_4bpp(0x01);
      pos += 2;
    }
  }

  if(color_depth == Background::Mode::BPP8) {
    uint pos = tile_num * 64;
    while(y--) {
      d0 = vram[pos +  0];
      d1 = vram[pos +  1];
      d2 = vram[pos + 16];
      d3 = vram[pos + 17];
      d4 = vram[pos + 32];
      d5 = vram[pos + 33];
      d6 = vram[pos + 48];
      d7 = vram[pos + 49];
      renderBGTile_line_8bpp(0x80);
      renderBGTile_line_8bpp(0x40);
      renderBGTile_line_8bpp(0x20);
      renderBGTile_line_8bpp(0x10);
      renderBGTile_line_8bpp(0x08);
      renderBGTile_line_8bpp(0x04);
      renderBGTile_line_8bpp(0x02);
      renderBGTile_line_8bpp(0x01);
      pos += 2;
    }
  }

  tiledataCache.tiledataState[color_depth][tile_num] = 0;

  #undef renderBGTile_line_2bpp
  #undef renderBGTile_line_4bpp
  #undef renderBGTile_line_8bpp
}

auto PPU::flushPixelCache() -> void {
  uint16 above = getPalette(0);
  uint16 below = (r.pseudoHires || r.bgMode == 5 || r.bgMode == 6) ? above : r.color_rgb;

  uint i = 255;
  do {
    pixelCache[i].aboveColor = above;
    pixelCache[i].belowColor = below;
    pixelCache[i].aboveLayer = Screen::ID::BACK;
    pixelCache[i].belowLayer = Screen::ID::BACK;
    pixelCache[i].aboveColorExemption = false;
    pixelCache[i].belowColorExemption = false;
    pixelCache[i].abovePriority = 0;
    pixelCache[i].belowPriority = 0;
  } while(i--);
}

auto PPU::TiledataCache::allocate() -> void {
  tiledata[Background::Mode::BPP2]      = new uint8[262144];
  tiledata[Background::Mode::BPP4]      = new uint8[131072];
  tiledata[Background::Mode::BPP8]      = new uint8[ 65536];
  tiledataState[Background::Mode::BPP2] = new uint8[  4096];
  tiledataState[Background::Mode::BPP4] = new uint8[  2048];
  tiledataState[Background::Mode::BPP8] = new uint8[  1024];
}

//marks all tiledata cache entries as dirty
auto PPU::TiledataCache::flush() -> void {
  for(uint i : range(4096)) tiledataState[Background::Mode::BPP2][i] = 1;
  for(uint i : range(2048)) tiledataState[Background::Mode::BPP4][i] = 1;
  for(uint i : range(1024)) tiledataState[Background::Mode::BPP8][i] = 1;
}

auto PPU::TiledataCache::free() -> void {
  delete[] tiledata[Background::Mode::BPP2];
  delete[] tiledata[Background::Mode::BPP4];
  delete[] tiledata[Background::Mode::BPP8];
  delete[] tiledataState[Background::Mode::BPP2];
  delete[] tiledataState[Background::Mode::BPP4];
  delete[] tiledataState[Background::Mode::BPP8];
}
