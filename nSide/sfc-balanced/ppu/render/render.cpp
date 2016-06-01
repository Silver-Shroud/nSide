#include "cache.cpp"
#include "windows.cpp"
#include "bg.cpp"
#include "oam.cpp"
#include "mode7.cpp"
#include "addsub.cpp"
#include "line.cpp"

//Mode 0: ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12
//  BG4B, BG3B, OAM0, BG4A, BG3A, OAM1, BG2B, BG1B, OAM2, BG2A, BG1A, OAM3
auto PPU::render_line_mode0() -> void {
  render_line_bg<0, Background::ID::BG1, Background::Mode::BPP2>(8, 11);
  render_line_bg<0, Background::ID::BG2, Background::Mode::BPP2>(7, 10);
  render_line_bg<0, Background::ID::BG3, Background::Mode::BPP2>(2,  5);
  render_line_bg<0, Background::ID::BG4, Background::Mode::BPP2>(1,  4);
  render_line_oam(3, 6, 9, 12);
}

//Mode 1 (pri=1): ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10
//  BG3B, OAM0, OAM1, BG2B, BG1B, OAM2, BG2A, BG1A, OAM3, BG3A
//
//Mode 1 (pri=0): ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10
//  BG3B, OAM0, BG3A, OAM1, BG2B, BG1B, OAM2, BG2A, BG1A, OAM3
auto PPU::render_line_mode1() -> void {
  if(regs.bg3_priority) {
    render_line_bg<1, Background::ID::BG1, Background::Mode::BPP4>(5,  8);
    render_line_bg<1, Background::ID::BG2, Background::Mode::BPP4>(4,  7);
    render_line_bg<1, Background::ID::BG3, Background::Mode::BPP2>(1, 10);
    render_line_oam(2, 3, 6, 9);
  } else {
    render_line_bg<1, Background::ID::BG1, Background::Mode::BPP4>(6,  9);
    render_line_bg<1, Background::ID::BG2, Background::Mode::BPP4>(5,  8);
    render_line_bg<1, Background::ID::BG3, Background::Mode::BPP2>(1,  3);
    render_line_oam(2, 4, 7, 10);
  }
}

//Mode 2: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OAM0, BG1B, OAM1, BG2A, OAM2, BG1A, OAM3
auto PPU::render_line_mode2() -> void {
  render_line_bg<2, Background::ID::BG1, Background::Mode::BPP4>(3, 7);
  render_line_bg<2, Background::ID::BG2, Background::Mode::BPP4>(1, 5);
  render_line_oam(2, 4, 6, 8);
}

//Mode 3: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OAM0, BG1B, OAM1, BG2A, OAM2, BG1A, OAM3
auto PPU::render_line_mode3() -> void {
  render_line_bg<3, Background::ID::BG1, Background::Mode::BPP8>(3, 7);
  render_line_bg<3, Background::ID::BG2, Background::Mode::BPP4>(1, 5);
  render_line_oam(2, 4, 6, 8);
}

//Mode 4: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OAM0, BG1B, OAM1, BG2A, OAM2, BG1A, OAM3
auto PPU::render_line_mode4() -> void {
  render_line_bg<4, Background::ID::BG1, Background::Mode::BPP8>(3, 7);
  render_line_bg<4, Background::ID::BG2, Background::Mode::BPP2>(1, 5);
  render_line_oam(2, 4, 6, 8);
}

//Mode 5: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OAM0, BG1B, OAM1, BG2A, OAM2, BG1A, OAM3
auto PPU::render_line_mode5() -> void {
  render_line_bg<5, Background::ID::BG1, Background::Mode::BPP4>(3, 7);
  render_line_bg<5, Background::ID::BG2, Background::Mode::BPP2>(1, 5);
  render_line_oam(2, 4, 6, 8);
}

//Mode 6: ->
//     1,    2,    3,    4,    5,    6
//  OAM0, BG1B, OAM1, OAM2, BG1A, OAM3
auto PPU::render_line_mode6() -> void {
  render_line_bg<6, Background::ID::BG1, Background::Mode::BPP4>(2, 5);
  render_line_oam(1, 3, 4, 6);
}

//Mode7: ->
//     1,    2,    3,    4,    5
//  OAM0, BG1n, OAM1, OAM2, OAM3

//Mode 7 EXTBG: ->
//     1,    2,    3,    4,    5,    6,    7
//  BG2B, OAM0, BG1n, OAM1, BG2A, OAM2, OAM3
auto PPU::render_line_mode7() -> void {
  if(!regs.mode7_extbg) {
    render_line_mode7<Background::ID::BG1>(2, 2);
    render_line_oam(1, 3, 4, 5);
  } else {
    render_line_mode7<Background::ID::BG1>(3, 3);
    render_line_mode7<Background::ID::BG2>(1, 5);
    render_line_oam(2, 4, 6, 7);
  }
}

auto PPU::render_line() -> void {
  render_line_oam_rto();
  if(regs.display_disable || line >= vdisp()) return render_line_clear();

  flush_pixel_cache();
  build_window_tables(COL);
  update_bg_info();

  switch(regs.bgmode) {
  case 0: render_line_mode0(); break;
  case 1: render_line_mode1(); break;
  case 2: render_line_mode2(); break;
  case 3: render_line_mode3(); break;
  case 4: render_line_mode4(); break;
  case 5: render_line_mode5(); break;
  case 6: render_line_mode6(); break;
  case 7: render_line_mode7(); break;
  }

  render_line_output();
}
