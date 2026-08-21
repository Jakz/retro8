#include "common.h"

#if TEST_MODE

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_NO_POSIX_SIGNALS
#include "catch.hpp"

#include "vm/machine.h"
#include "io/loader.h"
#include "lua/lua.hpp"

#include <unordered_set>
#include <filesystem>

using namespace retro8;
using namespace retro8::gfx;

extern retro8::Machine machine;
Machine& m = machine;

TEST_CASE("cursor([x,] [y,] [col])")
{
  auto* cursor = m.memory().cursor();

  SECTION("cursor starts at 0,0")
  {
    REQUIRE((cursor->x() == 0 && cursor->y() == 0));
  }
}

TEST_CASE("camera([x,] [y])")
{
  auto* camera = m.memory().camera();

  SECTION("camera starts at 0,0")
  {
    REQUIRE((camera->x() == 0 && camera->y() == 0));
  }

  SECTION("camera is properly set by camera() function")
  {
    m.code().initFromSource("camera(20,-10)");
    auto* camera = m.memory().camera();
    REQUIRE(camera->x() == 20);
    REQUIRE(camera->y() == -10);
  }

  SECTION("camera is properly reset by camera() function")
  {
    m.code().initFromSource("camera(20,-10)");
    REQUIRE((camera->x() == 20 && camera->y() == -10));
    m.code().initFromSource("camera()");
    REQUIRE((camera->x() == 0 && camera->y() == 0));
  }

  SECTION("single argument sets x and resets y")
  {
    m.code().initFromSource("camera(20,-10)");
    REQUIRE((camera->x() == 20 && camera->y() == -10));
    m.code().initFromSource("camera(40)");
    REQUIRE((camera->x() == 40 && camera->y() == 0));
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

TEST_CASE("mid")
{
  Machine m;

  SECTION("a < b < c = b")
  {
    m.code().initFromSource("function _test() return mid(1, 2, 3) end");
    m.code().callFunction("_test", 1);
    REQUIRE(lua_tonumber(m.code().state(), -1) == 2);
  }
}

TEST_CASE("bitwise")
{
  Machine m;

  std::string code;
  uint32_t expected;

  SECTION("and")
  {
    auto i = GENERATE(range(0, 255));
    auto j = GENERATE(range(0, 255));
    
    code = "return band(" + std::to_string(i) + ", " + std::to_string(j) + ")";
    expected = i & j;
  }

  SECTION("or")
  {
    auto i = GENERATE(range(0, 255));
    auto j = GENERATE(range(0, 255));

    code = "return bor(" + std::to_string(i) + ", " + std::to_string(j) + ")";
    expected = i | j;
  }

  m.code().initFromSource("function _test() " + code + " end");
  m.code().callFunction("_test", 1);
  REQUIRE(lua_tonumber(m.code().state(), -1) == expected);
}

TEST_CASE("PICO-8 API compatibility")
{
  Machine& testMachine = machine;
  testMachine.code().loadAPI();

  SECTION("tonum parses decimal strings")
  {
    testMachine.code().initFromSource("function _test() return tonum('123'), tonum('-4.5'), tonum('nope') end");
    testMachine.code().callFunction("_test", 3);
    REQUIRE(lua_tonumber(testMachine.code().state(), -3) == 123);
    REQUIRE(lua_tonumber(testMachine.code().state(), -2) == -4.5);
    REQUIRE(lua_isnil(testMachine.code().state(), -1));
  }

  SECTION("all tolerates deleting the current item")
  {
    testMachine.code().initFromSource("function _test() local t={1,2,3}; local out={}; for v in all(t) do add(out,v); if v==1 then del(t,v) end end return #out,out[1],out[2],out[3] end");
    testMachine.code().callFunction("_test", 4);
    REQUIRE(lua_tonumber(testMachine.code().state(), -4) == 3);
    REQUIRE(lua_tonumber(testMachine.code().state(), -3) == 1);
    REQUIRE(lua_tonumber(testMachine.code().state(), -2) == 2);
    REQUIRE(lua_tonumber(testMachine.code().state(), -1) == 3);
  }

  SECTION("coroutine helpers preserve arguments and yielded values")
  {
    testMachine.code().initFromSource("function _test() local co=cocreate(function(a) local b=yield(a+1); return b+2 end); local ok,a=coresume(co,4); local ok2,b=coresume(co,8); return ok,a,ok2,b end");
    testMachine.code().callFunction("_test", 4);
    REQUIRE(lua_toboolean(testMachine.code().state(), -4));
    REQUIRE(lua_tonumber(testMachine.code().state(), -3) == 5);
    REQUIRE(lua_toboolean(testMachine.code().state(), -2));
    REQUIRE(lua_tonumber(testMachine.code().state(), -1) == 10);
  }
}

TEST_CASE("PICO-8 boot graphics state")
{
  Machine& testMachine = machine;

  SECTION("default print color is visible after cls")
  {
    testMachine.font().load();
    testMachine.code().initFromSource("cls() print('x')");

    bool hasVisiblePixel = false;
    for (int y = 0; y < 128; ++y)
      for (int x = 0; x < 128; ++x)
        hasVisiblePixel = hasVisiblePixel || testMachine.pget(x, y) != color_t::BLACK;

    REQUIRE(hasVisiblePixel);
  }
}

TEST_CASE("PICO-8 lifecycle callbacks")
{
  Machine& testMachine = machine;

  SECTION("_init can install update and draw callbacks")
  {
    testMachine.font().load();
    testMachine.code().initFromSource("function _init() _update=function() pset(0,0,8) end _draw=function() pset(1,0,6) end end");

    REQUIRE(testMachine.code().hasInit());
    REQUIRE_FALSE(testMachine.code().hasUpdate());
    REQUIRE_FALSE(testMachine.code().hasDraw());

    testMachine.code().init();

    REQUIRE(testMachine.code().hasUpdate());
    REQUIRE(testMachine.code().hasDraw());

    testMachine.code().update();
    testMachine.code().draw();

    REQUIRE(testMachine.pget(0, 0) == color_t::RED);
    REQUIRE(testMachine.pget(1, 0) == color_t::LIGHT_GREY);
  }
}

TEST_CASE("lua language modifications")
{
  lua_State* L = luaL_newstate();

  SECTION("!= operator")
  {
    SECTION("!= operator is compiled successfully")
    {
      const std::string code = "x = 5 != 4";

      REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
      REQUIRE(lua_pcall(L, 0, 0, 0) == 0);
    }

  }

  SECTION("binary literals")
  {
    std::string literal;
    int expected = 0;
    
    SECTION("0b1")
    {
      literal = "0b1";
      expected = 0b1;
    }

    SECTION("0b100")
    {
      literal = "0b100";
      expected = 0b100;
    }

    SECTION("0b000")
    {
      literal = "0b000";
      expected = 0b000;
    }


    const std::string code = "x = " + literal + "; return x";
    REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
    REQUIRE(lua_pcall(L, 0, 1, 0) == 0);
    REQUIRE(lua_tonumber(L, -1) == expected);
  }

  SECTION("compound assignment")
  {
    SECTION("+= operator")
    {
      std::string code = "x = 2; x += 4; return x";
      REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
      REQUIRE(lua_pcall(L, 0, 1, 0) == 0);
      REQUIRE(lua_tonumber(L, -1) == 6);
    }


  }

  lua_close(L);
}

/*TEST_CASE("cartridge testing")
{
  retro8::io::Loader loader;
  retro8::Machine m;

  namespace fs = std::filesystem;
  std::error_code ec;
  
  for (const auto& p : fs::directory_iterator("cartridges", ec))
  {
    const auto& path = p.path();

    if (fs::is_regular_file(path) && path.extension() == ".p8")
    {
      SECTION(std::string("Test on source file ") + path.filename().generic_u8string())
      {
        lua_State* L = luaL_newstate();

        std::string code = loader.load(path.generic_u8string());
        int status = luaL_loadbufferx(L, code.c_str(), code.length(), path.filename().generic_u8string().c_str(), nullptr);

        if (status)
        {
          const char* message = "unknown error";
          if (lua_isstring(L, -1))
            message = lua_tostring(L, -1);
          FAIL("Error: " << message);
        }

        lua_close(L);
      }
    }
  }

}*/

int testMain(int rargc, char* rargv[])
{
  Catch::Session session;

  const char* argv2[] = { "retro8", "--success" };

  int argc = rargc;
  const char** argv = const_cast<const char**>(rargv);

  int result = session.run(argc, argv);
  return result;
}

#endif
