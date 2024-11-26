#include <tuw/debug.hpp>
#include <tuw/timing.hpp>

#include "gtest/gtest.h"

TEST(tuw, Debug)
{
  tuw::Timing timing;
  tuw::Debug debug;
  timing.start();
  std::cout << "Demo tuw_debug says: " << debug.info() << std::endl;
  timing.add("Info");
  timing.add(timing.plot());
  ASSERT_EQ(true, true);
}
