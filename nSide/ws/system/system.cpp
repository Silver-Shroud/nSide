#include <ws/ws.hpp>

namespace WonderSwan {

System system;
#include "io.cpp"
#include "video.cpp"
#include "serialization.cpp"

auto System::loaded() const -> bool { return _loaded; }
auto System::model() const -> Model { return _model; }
auto System::orientation() const -> bool { return _orientation; }
auto System::color() const -> bool { return r.color; }
auto System::planar() const -> bool { return r.format == 0; }
auto System::packed() const -> bool { return r.format == 1; }
auto System::depth() const -> bool { return r.color && r.depth == 1; }

auto System::init() -> void {
  assert(interface != nullptr);
}

auto System::term() -> void {
}

auto System::load(Model model) -> void {
  _model = model;

  interface->loadRequest(ID::SystemManifest, "manifest.bml", true);
  auto document = BML::unserialize(information.manifest);

  //note: IPLROM is currently undumped; otherwise we'd load it here ...

  if(auto node = document["system/eeprom"]) {
    eeprom.setName(node["name"].text());
    eeprom.setSize(node["size"].natural() / sizeof(uint16));
    eeprom.erase();
    //initialize user-data section
    for(uint addr = 0x0030; addr <= 0x003a; addr++) eeprom[addr] = 0x0000;
    interface->loadRequest(ID::SystemEEPROM, eeprom.name(), false);
  }

  cartridge.load();
  serializeInit();
  _orientation = cartridge.information.orientation;
  _loaded = true;
}

auto System::unload() -> void {
  if(!loaded()) return;

  eeprom.setName("");
  eeprom.setSize(0);

  cartridge.unload();
  _loaded = false;
}

auto System::power() -> void {
  Emulator::video.reset();
  Emulator::video.setInterface(interface);
  Emulator::video.resize(224, 224);
  configureVideoPalette();
  configureVideoEffects();

  Emulator::audio.reset();
  Emulator::audio.setInterface(interface);

  bus.power();
  iram.power();
  eeprom.power();
  cpu.power();
  ppu.power();
  apu.power();
  cartridge.power();
  scheduler.power();

  bus.map(this, 0x0060);
  bus.map(this, 0x00ba, 0x00be);

  r.unknown = 0;
  r.format = 0;
  r.depth = 0;
  r.color = 0;
}

auto System::run() -> void {
  scheduler.enter();
  pollKeypad();
}

auto System::runToSave() -> void {
  scheduler.synchronize(cpu.thread);
  scheduler.synchronize(ppu.thread);
  scheduler.synchronize(apu.thread);
  scheduler.synchronize(cartridge.thread);
}

auto System::pollKeypad() -> void {
  keypad.y1 = interface->inputPoll(0, _orientation, 0);
  keypad.y2 = interface->inputPoll(0, _orientation, 1);
  keypad.y3 = interface->inputPoll(0, _orientation, 2);
  keypad.y4 = interface->inputPoll(0, _orientation, 3);
  keypad.x1 = interface->inputPoll(0, _orientation, 4);
  keypad.x2 = interface->inputPoll(0, _orientation, 5);
  keypad.x3 = interface->inputPoll(0, _orientation, 6);
  keypad.x4 = interface->inputPoll(0, _orientation, 7);
  keypad.b = interface->inputPoll(0, _orientation, 8);
  keypad.a = interface->inputPoll(0, _orientation, 9);
  keypad.start = interface->inputPoll(0, _orientation, 10);

  if(keypad.y1 || keypad.y2 || keypad.y3 || keypad.y4
  || keypad.x1 || keypad.x2 || keypad.x3 || keypad.x4
  || keypad.b || keypad.a || keypad.start
  ) {
    cpu.raise(CPU::Interrupt::Input);
  }
}

auto System::rotate() -> void {
  _orientation = !_orientation;
  configureVideoEffects();
}

}
