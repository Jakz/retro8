#include "lua_bridge.h"

#include "machine.h"

#include <lua.hpp>
#include <iostream>

using namespace lua;

extern retro8::Machine machine;

int pset(lua_State* L)
{
  int args = lua_gettop(L);
  //TODO: check validity of arguments

  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int c;

  if (args == 3)
    c = lua_tonumber(L, 3);
  else
    c = machine.memory().penColor()->low();

  machine.pset(x, y, static_cast<retro8::color_t>(c));

  return 0;
}

int pget(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);

  lua_pushinteger(L, machine.pget(x, y));

  return 1;
}

int color(lua_State* L)
{
  int c = lua_tonumber(L, 1);

  machine.color(static_cast<retro8::color_t>(c));

  return 0;
}

int line(lua_State* L)
{
  int x0 = lua_tonumber(L, 1);
  int y0 = lua_tonumber(L, 2);
  int x1 = lua_tonumber(L, 3);
  int y1 = lua_tonumber(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.line(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

  return 0;
}

int rect(lua_State* L)
{
  int x0 = lua_tonumber(L, 1);
  int y0 = lua_tonumber(L, 2);
  int x1 = lua_tonumber(L, 3);
  int y1 = lua_tonumber(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.rect(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

  return 0;
}

// TODO: fill pattern on filled shaped
int rectfill(lua_State* L)
{
  int x0 = lua_tonumber(L, 1);
  int y0 = lua_tonumber(L, 2);
  int x1 = lua_tonumber(L, 3);
  int y1 = lua_tonumber(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.rectfill(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

  return 0;
}

int circ(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_tonumber(L, 3);

  int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circ(x, y, r, static_cast<retro8::color_t>(c));

  return 0;
}

int circfill(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_tonumber(L, 3);

  int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circfill(x, y, r, static_cast<retro8::color_t>(c));

  return 0;
}

int cls(lua_State* L)
{
  int c = lua_gettop(L) == 1 ? lua_tonumber(L, -1) : 0;

  machine.cls(static_cast<retro8::color_t>(c));

  return 0;
}

int spr(lua_State* L)
{
  assert(lua_isnumber(L, 2) && lua_isnumber(L, 3));

  int idx = lua_tonumber(L, 1);
  int x = lua_tonumber(L, 2);
  int y = lua_tonumber(L, 3);

  machine.spr(idx, x, y);

  return 0;
}

int pal(lua_State* L)
{
  int c0 = lua_tonumber(L, 1);
  int c1 = lua_tonumber(L, 2);

  machine.pal(static_cast<retro8::color_t>(c0), static_cast<retro8::color_t>(c1));

  return 0;
}

int camera(lua_State* L)
{
  //TODO: implement
  return 0;
}

int map(lua_State* L)
{
  //TODO: implement
  return 0;
}

int mget(lua_State* L)
{
  //TODO: implement
  lua_pushinteger(L, 0);
  return 1;
}

int print(lua_State* L)
{
  //TODO: optimize and use const char*?
  std::string text = lua_tostring(L, 1);
  int x = lua_tonumber(L, 2); //TODO: these are optional
  int y = lua_tonumber(L, 3);
  int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.print(text, x, y, static_cast<retro8::color_t>(c));

  return 0;
}

int debugprint(lua_State* L)
{
  std::string text = lua_tostring(L, 1);
  std::cout << text << std::endl;

  return 0;
}

namespace math
{
  using real_t = float;
  static constexpr float PI = 3.14159265358979323846;

  int cos(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t angle = lua_tonumber(L, 1);
    real_t value = ::cos(angle * 2 * PI);
    lua_pushnumber(L, value);

    return 1;
  }

  int sin(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t angle = lua_tonumber(L, 1);
    real_t value = ::sin(-angle * 2 * PI);
    lua_pushnumber(L, value);

    return 1;
  }

  int srand(lua_State* L)
  {
    assert(lua_gettop(L) == 1);
    assert(lua_isnumber(L, 1));

    real_t seed = lua_tonumber(L, 1);
    machine.state().rnd.seed(seed);

    return 0;
  }

  int rnd(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t max = lua_tonumber(L, 1);
    lua_pushnumber(L, (machine.state().rnd() / (float)machine.state().rnd.max()) * max);

    return 1;
  }

  int flr(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t value = lua_tonumber(L, 1);
    lua_pushnumber(L, ::floor(value));

    return 1;
  }

  int min(lua_State* L)
  {
    assert(lua_isnumber(L, 1));
    assert(lua_isnumber(L, 2));

    real_t v1 = lua_tonumber(L, 1), v2 = lua_tonumber(L, 2);
    lua_pushnumber(L, std::min(v1, v2));

    return 1;
  }
}

namespace sound
{
  int music(lua_State* L)
  {
    //TODO: implement
    return 0;
  }

  int sfx(lua_State* L)
  {
    //TODO: implement
    return 0;
  }
}

namespace platform
{
  int btn(lua_State* L)
  {
    /* we're asking for a specific button*/
    if (lua_gettop(L) >= 1)
    {
      using bt_t = retro8::button_t;
      static constexpr std::array<bt_t, 6> buttons = { bt_t::LEFT, bt_t::RIGHT, bt_t::UP, bt_t::DOWN, bt_t::ACTION1, bt_t::ACTION2 };
      lua_pushboolean(L, machine.state().buttons.isSet(buttons[(int)lua_tonumber(L, 1)]));
    }
    /* push whole bitmask*/
    else
    {
      lua_pushnumber(L, machine.state().buttons.value);
    }
    
    //TODO: finish for player 2?
    return 1;
  }

  int btnp(lua_State* L)
  {
    //TODO: check behavior

    using bt_t = retro8::button_t;
    bit_mask<bt_t> changedButtons = machine.state().buttons & ~machine.state().previousButtons;
    
    /* we're asking for a specific button*/
    if (lua_gettop(L) >= 1)
    {
      static constexpr std::array<bt_t, 6> buttons = { bt_t::LEFT, bt_t::RIGHT, bt_t::UP, bt_t::DOWN, bt_t::ACTION1, bt_t::ACTION2 };
      lua_pushboolean(L, changedButtons.isSet(buttons[(int)lua_tonumber(L, 1)]));
    }
    /* push whole bitmask*/
    else
    {
      lua_pushnumber(L, changedButtons.value);
    }

    //TODO: finish for player?
    return 1;
  }
}

void lua::registerFunctions(lua_State* L)
{
  lua_register(L, "pset", pset);
  lua_register(L, "pget", pget);
  lua_register(L, "pal", pal);
  lua_register(L, "color", color);
  lua_register(L, "line", line);
  lua_register(L, "rect", rect);
  lua_register(L, "rectfill", rectfill);
  lua_register(L, "circ", circ);
  lua_register(L, "circfill", circfill);
  lua_register(L, "cls", cls);
  lua_register(L, "spr", spr);
  lua_register(L, "camera", camera);
  lua_register(L, "map", map);
  lua_register(L, "mget", mget);
  lua_register(L, "print", print);

  lua_register(L, "debug", debugprint);

  lua_register(L, "cos", math::cos);
  lua_register(L, "sin", math::sin);
  lua_register(L, "srand", math::srand);
  lua_register(L, "rnd", math::rnd);
  lua_register(L, "flr", math::flr);
  lua_register(L, "min", math::min);

  lua_register(L, "music", sound::music);
  lua_register(L, "sfx", sound::music);

  lua_register(L, "btn", platform::btn);
  lua_register(L, "btnp", platform::btnp);
}

Code::~Code()
{
  if (L)
    lua_close(L);
}

void Code::initFromSource(const std::string& code)
{
  L = luaL_newstate();

  luaopen_base(L);
  luaopen_table(L);

  registerFunctions(L);

  if (luaL_loadstring(L, code.c_str()))
  {
    const char* message = lua_tostring(L, -1);
    std::cout << "Error: on loadstring: " << message << std::endl;
    getchar();
  }

  int error = lua_pcall(L, 0, 0, 0);

  if (error)
  {
    const char* message = lua_tostring(L, -1);

    std::cout << "Error: script not loaded " << message << std::endl;
    getchar();
  }

  lua_getglobal(L, "_update");
  if (lua_isfunction(L, -1))
  {
    _update = lua_topointer(L, -1);
    lua_pop(L, 1);
  }

  lua_getglobal(L, "_update60");

  if (lua_isfunction(L, -1))
  {
    _update60 = lua_topointer(L, -1);
    lua_pop(L, 1);
  }

  lua_getglobal(L, "_draw");

  if (lua_isfunction(L, -1))
  {
    _draw = lua_topointer(L, -1);
    lua_pop(L, 1);
  }

  lua_getglobal(L, "_init");

  if (lua_isfunction(L, -1))
  {
    _init = lua_topointer(L, -1);
    lua_pop(L, 1);
  }
}

void Code::callVoidFunction(const char* name)
{
  lua_getglobal(L, name);
  int error = lua_pcall(L, 0, 0, 0);

  if (error)
  {
    const char* message = lua_tostring(L, -1);

    std::cout << "Error in " << name << " function: " << message << std::endl;
    getchar();
  }
}

void Code::update()
{

  if (_update60)
    callVoidFunction("_update60");
  else if (_update)
    callVoidFunction("_update");
}

void Code::draw()
{
  if (_draw)
    callVoidFunction("_draw");
}

void Code::init()
{
  if (_init)
    callVoidFunction("_init");
}