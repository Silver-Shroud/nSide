struct Expansion : Cothread {
  Expansion();
  static auto Enter() -> void;
  virtual auto main() -> void;
};

#include <sfc-balanced/expansion/satellaview/satellaview.hpp>
#include <sfc-balanced/expansion/superdisc/superdisc.hpp>
#include <sfc-balanced/expansion/21fx/21fx.hpp>