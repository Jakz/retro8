#include "common.h"

#if TEST_MODE

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "vm/machine.h"

using namespace retro8;

TEST_CASE("cursor([x,] [y,] [col])")
{
  SECTION("cursor starts at 0,0")
  {
    Machine m;
    REQUIRE(m.memory().cursor()->x == 0);
    REQUIRE(m.memory().cursor()->y == 0);
  }

  SECTION("cursor is correctly set by cursor function")
  {

  }
}

int testMain(int argc, char* argv[])
{
  int result = Catch::Session().run(argc, argv);
  return result;
}

#endif