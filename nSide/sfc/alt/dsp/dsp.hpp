#include "../../dsp/audio.hpp"
#include "SPC_DSP.h"

struct DSP : Thread {
  enum : bool { Threaded = false };

  DSP();

  alwaysinline auto step(uint clocks) -> void;
  alwaysinline auto synchronizeSMP() -> void;

  auto mute() -> bool;
  auto read(uint8_t addr) -> uint8_t;
  auto write(uint8_t addr, uint8_t data) -> void;

  auto main() -> void;
  auto power() -> void;
  auto reset() -> void;

  auto channel_enable(uint channel, bool enable) -> void;

  auto serialize(serializer&) -> void;

private:
  SPC_DSP spc_dsp;
  int16_t samplebuffer[8192];
  bool channel_enabled[8];
};

extern DSP dsp;
