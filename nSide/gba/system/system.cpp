#include <gba/gba.hpp>

namespace GameBoyAdvance {

#include "bios.cpp"
#include "video.cpp"
#include "serialization.cpp"
BIOS bios;
System system;
Scheduler scheduler;
Cheat cheat;

auto System::run() -> void {
  if(scheduler.enter() == Scheduler::Event::Frame) ppu.refresh();
}

auto System::runToSave() -> void {
  scheduler.synchronize(cpu);
  scheduler.synchronize(ppu);
  scheduler.synchronize(apu);
}

auto System::load(Emulator::Interface* interface) -> bool {
  if(auto fp = platform->open(ID::System, "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else return false;

  auto document = BML::unserialize(information.manifest);

  if(auto name = document["system/cpu/rom/name"].text()) {
    if(auto fp = platform->open(ID::System, name, File::Read, File::Required)) {
      fp->read(bios.data, bios.size);
    }
  }

  if(!cartridge.load()) return false;

  serializeInit();
  _orientation = 0;
  this->interface = interface;
  return _loaded = true;
}

auto System::save() -> void {
  if(!loaded()) return;
  cartridge.save();
}

auto System::unload() -> void {
  if(!loaded()) return;
  cartridge.unload();
  _loaded = false;
}

auto System::power() -> void {
  Emulator::video.reset();
  Emulator::video.setInterface(interface);
  Emulator::video.resize(240, 240);
  configureVideoPalette();
  configureVideoEffects();

  Emulator::audio.reset();
  Emulator::audio.setInterface(interface);

  scheduler.reset();
  bus.power();
  player.power();
  cpu.power();
  ppu.power();
  apu.power();
  cartridge.power();
  scheduler.primary(cpu);
}

auto System::rotate() -> void {
  _orientation = !_orientation;
  configureVideoEffects();
}

}
