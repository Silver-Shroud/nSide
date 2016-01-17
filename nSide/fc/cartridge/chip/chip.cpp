#include "fcg.cpp"
#include "g101.cpp"
#include "ifh3001.cpp"
#include "mmc1.cpp"
#include "mmc3.cpp"
#include "mmc5.cpp"
#include "mmc6.cpp"
#include "n108.cpp"
#include "n163.cpp"
#include "ss88006.cpp"
#include "tc.cpp"
#include "vrc1.cpp"
#include "vrc2.cpp"
#include "vrc3.cpp"
#include "vrc4.cpp"
#include "vrc6.cpp"
#include "vrc7.cpp"

auto Chip::tick() -> void {
  board.tick();
}

Chip::Chip(Board& board) : board(board) {
}