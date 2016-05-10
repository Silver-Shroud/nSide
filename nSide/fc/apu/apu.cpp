#include <fc/fc.hpp>

namespace Famicom {

APU apu;

#include "envelope.cpp"
#include "sweep.cpp"
#include "pulse.cpp"
#include "triangle.cpp"
#include "noise.cpp"
#include "dmc.cpp"
#include "serialization.cpp"

APU::APU() {
  for(uint amp : range(32)) {
    if(amp == 0) {
      pulse_dac[amp] = 0;
    } else {
      pulse_dac[amp] = 16384.0 * 95.88 / (8128.0 / amp + 100.0);
    }
  }

  for(uint dmc_amp : range(128)) {
    for(uint triangle_amp : range(16)) {
      for(uint noise_amp : range(16)) {
        if(dmc_amp == 0 && triangle_amp == 0 && noise_amp == 0) {
          dmc_triangle_noise_dac[dmc_amp][triangle_amp][noise_amp] = 0;
        } else {
          dmc_triangle_noise_dac[dmc_amp][triangle_amp][noise_amp]
          = 16384.0 * 159.79 / (100.0 + 1.0 / (triangle_amp / 8227.0 + noise_amp / 12241.0 + dmc_amp / 22638.0));
        }
      }
    }
  }
}

auto APU::Enter() -> void {
  while(true) scheduler.synchronize(), apu.main();
}

auto APU::main() -> void {
  uint pulse_output, triangle_output, noise_output, dmc_output;

  pulse_output  = pulse[0].clock();
  pulse_output += pulse[1].clock();
  triangle_output = triangle.clock();
  noise_output = noise.clock();
  dmc_output = dmc.clock();

  clock_frame_counter_divider();

  int output = pulse_dac[pulse_output] + dmc_triangle_noise_dac[dmc_output][triangle_output][noise_output];

  output  = filter.run_hipass_strong(output);
  output += cartridge_sample;
  output  = filter.run_hipass_weak(output);
//output  = filter.run_lopass(output);
  output  = sclamp<16>(output);

  stream->sample(output, output);

  tick();
}

auto APU::tick() -> void {
  switch(system.region()) {
  case System::Region::NTSC:  clock += 12; break;
  case System::Region::PAL:   clock += 16; break;
  case System::Region::Dendy: clock += 15; break;
  }
  if(clock >= 0 && !scheduler.synchronizing()) co_switch(cpu.thread);
}

auto APU::set_irq_line() -> void {
  cpu.set_irq_apu_line(frame.irq_pending || dmc.irq_pending);
}

auto APU::set_sample(int16 sample) -> void {
  cartridge_sample = sample;
}

auto APU::power() -> void {
  filter.hipass_strong = 0;
  filter.hipass_weak = 0;
  filter.lopass = 0;

  pulse[0].power();
  pulse[1].power();
  triangle.power();
  noise.power();
  dmc.power();
}

auto APU::reset() -> void {
  create(APU::Enter, system.cpuFrequency());
  double clockDivider;
  switch(system.region()) {
  case System::Region::NTSC:  clockDivider = 12.0; break;
  case System::Region::PAL:   clockDivider = 16.0; break;
  case System::Region::Dendy: clockDivider = 15.0; break;
  }
  stream = Emulator::audio.createStream(1, system.cpuFrequency() / clockDivider);

  pulse[0].reset();
  pulse[1].reset();
  triangle.reset();
  noise.reset();
  dmc.reset();

  frame.irq_pending = 0;

  frame.mode = 0;
  frame.counter = 0;
  frame.divider = 1;

  enabled_channels = 0;
  cartridge_sample = 0;

  set_irq_line();
}

auto APU::read(uint16 addr) -> uint8 {
  if(addr == 0x4015) {
    uint8 result = 0x00;
    result |= pulse[0].length_counter ? 0x01 : 0;
    result |= pulse[1].length_counter ? 0x02 : 0;
    result |= triangle.length_counter ? 0x04 : 0;
    result |=    noise.length_counter ? 0x08 : 0;
    result |=      dmc.length_counter ? 0x10 : 0;
    result |=       frame.irq_pending ? 0x40 : 0;
    result |=         dmc.irq_pending ? 0x80 : 0;

    frame.irq_pending = false;
    set_irq_line();

    return result;
  }

  return cpu.mdr();
}

auto APU::write(uint16 addr, uint8 data) -> void {
  const uint n = addr.bit(2);  //pulse#

  switch(addr) {
  case 0x4000: case 0x4004:
    pulse[n].duty = data.bits(6,7);
    pulse[n].envelope.loop_mode = data.bit(5);
    pulse[n].envelope.use_speed_as_volume = data.bit(4);
    pulse[n].envelope.speed = data.bits(0,3);
    break;

  case 0x4001: case 0x4005:
    pulse[n].sweep.enable = data.bit(7);
    pulse[n].sweep.period = data.bits(4,6);
    pulse[n].sweep.decrement = data.bit(3);
    pulse[n].sweep.shift = data.bits(0,2);
    pulse[n].sweep.reload = true;
    break;

  case 0x4002: case 0x4006:
    pulse[n].period = (pulse[n].period & 0x0700) | (data << 0);
    pulse[n].sweep.pulse_period = (pulse[n].sweep.pulse_period & 0x0700) | (data << 0);
    break;

  case 0x4003: case 0x4007:
    pulse[n].period = (pulse[n].period & 0x00ff) | (data << 8);
    pulse[n].sweep.pulse_period = (pulse[n].sweep.pulse_period & 0x00ff) | (data << 8);

    pulse[n].duty_counter = 7;
    pulse[n].envelope.reload_decay = true;

    if(enabled_channels & (1 << n)) {
      pulse[n].length_counter = length_counter_table[data.bits(3,7)];
    }
    break;

  case 0x4008:
    triangle.halt_length_counter = data.bit(7);
    triangle.linear_length = data.bits(0,6);
    break;

  case 0x400a:
    triangle.period = (triangle.period & 0x0700) | (data << 0);
    break;

  case 0x400b:
    triangle.period = (triangle.period & 0x00ff) | (data << 8);

    triangle.reload_linear = true;

    if(enabled_channels.bit(2)) triangle.length_counter = length_counter_table[data.bits(3,7)];
    break;

  case 0x400c:
    noise.envelope.loop_mode = data.bit(5);
    noise.envelope.use_speed_as_volume = data.bit(4);
    noise.envelope.speed = data.bits(0,3);
    break;

  case 0x400e:
    //TODO: Check if the RP2A03E and prior revisions support short mode.
    //Currently assuming that the RP2A03F is bugged. See noise.cpp for bug implementation.
    if(revision != Revision::RP2A03) noise.short_mode = data.bit(7);
    noise.period = data.bits(0,3);
    break;

  case 0x400f:
    noise.envelope.reload_decay = true;

    if(enabled_channels.bit(3)) noise.length_counter = length_counter_table[data.bits(3,7)];
    break;

  case 0x4010:
    dmc.irq_enable = data.bit(7);
    dmc.loop_mode = data.bit(6);
    dmc.period = data.bits(0,3);

    dmc.irq_pending = dmc.irq_pending && dmc.irq_enable && !dmc.loop_mode;
    set_irq_line();
    break;

  case 0x4011:
    dmc.dac_latch = data.bits(0,6);
    break;

  case 0x4012:
    dmc.addr_latch = data;
    break;

  case 0x4013:
    dmc.length_latch = data;
    break;

  case 0x4015:
    if(!data.bit(0)) pulse[0].length_counter = 0;
    if(!data.bit(1)) pulse[1].length_counter = 0;
    if(!data.bit(2)) triangle.length_counter = 0;
    if(!data.bit(3))    noise.length_counter = 0;

    data.bit(4) ? dmc.start() : dmc.stop();
    dmc.irq_pending = false;

    set_irq_line();
    enabled_channels = data.bits(0,4);
    break;

  case 0x4017:
    frame.mode = data.bits(6,7);

    frame.counter = 0;
    if(frame.mode.bit(1)) clock_frame_counter();
    if(frame.mode.bit(0)) {
      frame.irq_pending = false;
      set_irq_line();
    }
    frame.divider = system.region() == System::Region::NTSC ? FrameCounter::NtscPeriod : FrameCounter::PalPeriod;
    break;
  }
}

auto APU::Filter::run_hipass_strong(int sample) -> int {
  hipass_strong += ((((int64)sample << 16) - (hipass_strong >> 16)) * HiPassStrong) >> 16;
  return sample - (hipass_strong >> 32);
}

auto APU::Filter::run_hipass_weak(int sample) -> int {
  hipass_weak += ((((int64)sample << 16) - (hipass_weak >> 16)) * HiPassWeak) >> 16;
  return sample - (hipass_weak >> 32);
}

auto APU::Filter::run_lopass(int sample) -> int {
  lopass += ((((int64)sample << 16) - (lopass >> 16)) * LoPass) >> 16;
  return (lopass >> 32);
}

auto APU::clock_frame_counter() -> void {
  frame.counter++;

  if(frame.counter & 1) {
    pulse[0].clock_length();
    pulse[0].sweep.clock(0);
    pulse[1].clock_length();
    pulse[1].sweep.clock(1);
    triangle.clock_length();
    noise.clock_length();
  }

  pulse[0].envelope.clock();
  pulse[1].envelope.clock();
  triangle.clock_linear_length();
  noise.envelope.clock();

  if(frame.counter == 0) {
    if(frame.mode & 2) {
      if(system.region() == System::Region::NTSC)
        frame.divider += FrameCounter::NtscPeriod;
      else
        frame.divider += FrameCounter::PalPeriod;
    }
    if(frame.mode == 0) {
      frame.irq_pending = true;
      set_irq_line();
    }
  }
}

auto APU::clock_frame_counter_divider() -> void {
  frame.divider -= 2;
  if(frame.divider <= 0) {
    clock_frame_counter();
    if(system.region() == System::Region::NTSC)
      frame.divider += FrameCounter::NtscPeriod;
    else
      frame.divider += FrameCounter::PalPeriod;
  }
}

const uint8 APU::length_counter_table[32] = {
  0x0a, 0xfe, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xa0, 0x08, 0x3c, 0x0a, 0x0e, 0x0c, 0x1a, 0x0e,
  0x0c, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xc0, 0x18, 0x48, 0x1a, 0x10, 0x1c, 0x20, 0x1e,
};

const uint16 APU::ntsc_noise_period_table[16] = {
  4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

const uint16 APU::pal_noise_period_table[16] = {
  4, 7, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778,
};

const uint16 APU::ntsc_dmc_period_table[16] = {
  428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54,
};

const uint16 APU::pal_dmc_period_table[16] = {
  398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118,  98, 78, 66, 50,
};

}
