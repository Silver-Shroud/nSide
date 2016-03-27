struct Vaus : Controller {
  enum : uint {
    Control, Fire,
  };

  Vaus(uint port);

  auto main() -> void;
  auto data() -> uint5;
  auto data1() -> uint2;
  auto data2() -> uint5;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  int x;
  uint8 control;

  uint prev;
};