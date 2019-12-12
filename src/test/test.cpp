#include "common.h"

#if TEST_MODE

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "vm/machine.h"

#include <unordered_set>

using namespace retro8;
using namespace retro8::gfx;

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

TEST_CASE("tilemap")
{
  Machine m;

  SECTION("tilemap address space never overlap")
  {
    std::unordered_set<const sprite_index_t*> addresses;

    for (size_t y = 0; y < TILE_MAP_HEIGHT; ++y)
    {
      for (size_t x = 0; x < TILE_MAP_WIDTH; ++x)
      {
        const sprite_index_t* address = m.memory().spriteInTileMap(x, y);
        addresses.insert(address);
      }
    }

    REQUIRE(addresses.size() == TILE_MAP_WIDTH * TILE_MAP_HEIGHT);
  }
  
  SECTION("tilemap halves are inverted compared to theie coordinates")
  {
    REQUIRE(m.memory().spriteInTileMap(0, 32) - m.memory().base() == address::TILE_MAP_LOW);
    REQUIRE(m.memory().spriteInTileMap(0, 0) - m.memory().base() == address::TILE_MAP_HIGH);
    REQUIRE(address::TILE_MAP_HIGH > address::TILE_MAP_LOW);
  }


}

int testMain(int argc, char* argv[])
{
  int result = Catch::Session().run(argc, argv);
  return result;
}

#endif