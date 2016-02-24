#include <sfc/sfc.hpp>

namespace SuperFamicom {

DSP dsp;
#include "../../dsp/audio.cpp"

#include "SPC_DSP.cpp"
#include "serialization.cpp"

DSP::DSP() {
  for(auto i : range(8)) channel_enabled[i] = true;
}

auto DSP::step(uint_t clocks) -> void {
  clock += clocks;
}

auto DSP::synchronizeSMP() -> void {
  if(SMP::Threaded) {
    if(clock >= 0 && !scheduler.synchronizing()) co_switch(smp.thread);
  } else {
    while(clock >= 0) smp.main();
  }
}

auto DSP::main() -> void {
  spc_dsp.run(1);
  step(24);

  int_t count = spc_dsp.sample_count();
  if(count > 0) {
    for(uint_t n = 0; n < count; n += 2) audio.sample(samplebuffer[n + 0], samplebuffer[n + 1]);
    spc_dsp.set_output(samplebuffer, 8192);
  }
}

auto DSP::mute() -> bool {
  return spc_dsp.mute();
}

auto DSP::read(uint8_t addr) -> uint8_t {
  return spc_dsp.read(addr);
}

auto DSP::write(uint8_t addr, uint8_t data) -> void {
  spc_dsp.write(addr, data);
}

auto DSP::power() -> void {
  spc_dsp.init(smp.apuram);
  spc_dsp.reset();
  spc_dsp.set_output(samplebuffer, 8192);

  audio.coprocessorEnable(false);
}

auto DSP::reset() -> void {
  Thread::clock = 0;
  spc_dsp.soft_reset();
  spc_dsp.set_output(samplebuffer, 8192);
}

auto DSP::channel_enable(uint_t channel, bool enable) -> void {
  channel_enabled[channel & 7] = enable;
  uint_t mask = 0;
  for(auto i : range(8)) {
    if(channel_enabled[i] == false) mask |= 1 << i;
  }
  spc_dsp.mute_voices(mask);
}

}
