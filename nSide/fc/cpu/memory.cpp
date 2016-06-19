auto CPU::read(uint16 addr) -> uint8 {
  if(status.oamdmaPending) {
    status.oamdmaPending = false;
    read(addr);
    oamdma();
  }

  while(status.rdyLine == 0) {
    r.mdr = bus.read(status.rdyAddrValid ? status.rdyAddrValue : addr, r.mdr);
    addClocks(system.region() == System::Region::NTSC ? 12 : system.region() == System::Region::PAL ? 16 : 15);
  }

  r.mdr = bus.read(addr, r.mdr);
  addClocks(system.region() == System::Region::NTSC ? 12 : system.region() == System::Region::PAL ? 16 : 15);
  return r.mdr;
}

auto CPU::write(uint16 addr, uint8 data) -> void {
  bus.write(addr, r.mdr = data);
  addClocks(system.region() == System::Region::NTSC ? 12 : system.region() == System::Region::PAL ? 16 : 15);
}

auto CPU::disassemblerRead(uint16 addr) -> uint8 {
  return bus.read(addr, r.mdr);
}
