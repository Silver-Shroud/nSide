#include "../laevateinn.hpp"
unique_pointer<Debugger> debugger;

#include "hook.cpp"
#include "usage.cpp"

Debugger::Debugger() {
  debugger = this;
  paused = true;

  flags.step = false;
  flags.cpu.stepInto = false;
  flags.cpu.nmi = false;
  flags.cpu.irq = false;
  flags.smp.stepInto = false;

  debug.cpu = true;
  debug.smp = false;

  cpuUsage.allocate(16 * 1024 * 1024);
  apuUsage.allocate(64 * 1024);

  SuperFamicom::cpu.debugger.op_exec = {&Debugger::cpu_op_exec, this};
  SuperFamicom::cpu.debugger.op_read = {&Debugger::cpu_op_read, this};
  SuperFamicom::cpu.debugger.op_write = {&Debugger::cpu_op_write, this};

  SuperFamicom::cpu.debugger.op_nmi = {&Debugger::cpu_op_nmi, this};
  SuperFamicom::cpu.debugger.op_irq = {&Debugger::cpu_op_irq, this};

  SuperFamicom::smp.debugger.op_exec = {&Debugger::smp_op_exec, this};
  SuperFamicom::smp.debugger.op_read = {&Debugger::smp_op_read, this};
  SuperFamicom::smp.debugger.op_write = {&Debugger::smp_op_write, this};

  SuperFamicom::ppu.debugger.vramRead = {&Debugger::ppu_vramRead, this};
  SuperFamicom::ppu.debugger.vramWrite = {&Debugger::ppu_vramWrite, this};

  SuperFamicom::ppu.debugger.oamRead = {&Debugger::ppu_oamRead, this};
  SuperFamicom::ppu.debugger.oamWrite = {&Debugger::ppu_oamWrite, this};

  SuperFamicom::ppu.debugger.cgramRead = {&Debugger::ppu_cgramRead, this};
  SuperFamicom::ppu.debugger.cgramWrite = {&Debugger::ppu_cgramWrite, this};
}

auto Debugger::run() -> void {
  if(paused == true) {
    usleep(2000);
    return;
  }

  emulator->run();
  if(cpuDebugger->autoUpdate.checked()) cpuDebugger->updateDisassembly();
  if(smpDebugger->autoUpdate.checked()) smpDebugger->updateDisassembly();
  if(memoryEditor->autoUpdate.checked()) memoryEditor->updateView();
  if(propertiesViewer->autoUpdate.checked()) propertiesViewer->updateProperties();
  if(vramViewer->autoUpdate.checked()) vramViewer->updateTiles();
  if(bgViewer->autoUpdate.checked()) bgViewer->updateTiles();
  if(paletteViewer->autoUpdate.checked()) paletteViewer->updateColors();
}

auto Debugger::echo(const string& text) -> void {
  consoleWindow->print(text);
}

auto Debugger::resume() -> void {
  if(paused == false) return;
  paused = false;
  consoleWindow->runButton.setText("Stop");
}

auto Debugger::suspend() -> void {
  if(paused == true) return;
  paused = true;
  flags.step = false;
  flags.cpu.stepInto = false;
  flags.cpu.nmi = false;
  flags.cpu.irq = false;
  flags.smp.stepInto = false;
  consoleWindow->runButton.setText("Run");
}
