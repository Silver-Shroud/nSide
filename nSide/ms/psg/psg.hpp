//TI SN76489

struct PSG : Thread {
  shared_pointer<Emulator::Stream> stream;

  static auto Enter() -> void;
  auto main() -> void;
  auto step(uint clocks) -> void;

  auto out(uint8 addr, uint8 data) -> void;

  auto power() -> void;
  auto reset() -> void;
};

extern PSG psg;
