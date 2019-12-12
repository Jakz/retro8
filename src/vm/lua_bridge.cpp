#include "lua_bridge.h"

#include "machine.h"

#include <lua.hpp>
#include <iostream>

using namespace lua;
using namespace retro8;

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

  machine.pset(x, y, static_cast<color_t>(c));

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

  machine.color(static_cast<color_t>(c));

  return 0;
}

int line(lua_State* L)
{
  int x0 = lua_tonumber(L, 1);
  int y0 = lua_tonumber(L, 2);
  int x1 = lua_tonumber(L, 3);
  int y1 = lua_tonumber(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.line(x0, y0, x1, y1, static_cast<color_t>(c));

  return 0;
}

int rect(lua_State* L)
{
  int x0 = lua_tonumber(L, 1);
  int y0 = lua_tonumber(L, 2);
  int x1 = lua_tonumber(L, 3);
  int y1 = lua_tonumber(L, 4);

  int c = lua_gettop(L) == 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.rect(x0, y0, x1, y1, static_cast<color_t>(c));

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

  machine.rectfill(x0, y0, x1, y1, static_cast<color_t>(c));

  return 0;
}

int circ(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_tonumber(L, 3);

  int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circ(x, y, r, static_cast<color_t>(c));

  return 0;
}

int circfill(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_tonumber(L, 3);

  int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circfill(x, y, r, static_cast<color_t>(c));

  return 0;
}

int cls(lua_State* L)
{
  int c = lua_gettop(L) == 1 ? lua_tonumber(L, -1) : 0;

  machine.cls(static_cast<color_t>(c));

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

  //TODO: third parameter to decide which palette

  machine.pal(static_cast<color_t>(c0), static_cast<color_t>(c1));

  return 0;
}

namespace draw
{
  int clip(lua_State* L)
  {
    if (lua_gettop(L) == 0)
      machine.memory().clipRect()->reset();
    else
    {
      uint8_t x0 = lua_tonumber(L, 1);
      uint8_t y0 = lua_tonumber(L, 2);
      uint8_t w = lua_tonumber(L, 3);
      uint8_t h = lua_tonumber(L, 4);

      machine.memory().clipRect()->set(x0, y0, x0 + w, y0 + h);
    }

    return 0;
  }
}



int camera(lua_State* L)
{
  int16_t cx = lua_tonumber(L, 1);
  int16_t cy = lua_tonumber(L, 2);
  machine.memory().camera()->set(cx, cy);

  return 0;
}

int map(lua_State* L)
{
  coord_t cx = lua_tonumber(L, 1);
  coord_t cy = lua_tonumber(L, 2);
  coord_t x = lua_tonumber(L, 3);
  coord_t y = lua_tonumber(L, 4);
  amount_t cw = lua_tonumber(L, 5);
  amount_t ch = lua_tonumber(L, 6);
  sprite_flags_t layer = 0;

  if (lua_gettop(L) == 7)
    layer = lua_tonumber(L, 7);

  machine.map(cx, cy, x, y, cw, ch, layer);

  return 0;
}

int mget(lua_State* L)
{
  int x = lua_tonumber(L, 1); //TODO: these are optional
  int y = lua_tonumber(L, 2);

  sprite_index_t index = 0;

  if (x >= 0 && x <= gfx::TILE_MAP_WIDTH && y >= 0 && y < gfx::TILE_MAP_HEIGHT)
    index = *machine.memory().spriteInTileMap(x, y);

  //printf("mget(%d, %d) = %d\n", x, y, index);

  lua_pushnumber(L, index);

  return 1;
}

int mset(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  retro8::sprite_index_t index = lua_tonumber(L, 3);

  *machine.memory().spriteInTileMap(x, y) = index;

  return 0;
}

int print(lua_State* L)
{
  //TODO: optimize and use const char*?
  std::string text = lua_tostring(L, 1);

  if (lua_gettop(L) == 1)
  {
    auto* cursor = machine.memory().cursor();

    retro8::coord_t x = cursor->x;
    retro8::coord_t y = cursor->y;
    retro8::color_t c = machine.memory().penColor()->low();
    machine.print(text, x, y, c);
    cursor->y += 6; //TODO: check height / magic number
  }
  else if (lua_gettop(L) >= 3)
  {
    int x = lua_tonumber(L, 2); //TODO: these are optional
    int y = lua_tonumber(L, 3);
    int c = lua_gettop(L) == 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

    machine.print(text, x, y, static_cast<retro8::color_t>(c));
  }
  else
    assert(false);


  return 0;
}

int cursor(lua_State* L)
{
  if (lua_gettop(L) >= 2)
  {
    int x = lua_tonumber(L, 1);
    int y = lua_tonumber(L, 2);
    *machine.memory().cursor() = { (uint8_t)x, (uint8_t)y };

    if (lua_gettop(L) == 3)
    {
      retro8::color_t color = static_cast<retro8::color_t>((int)lua_tonumber(L, 2));
      machine.memory().penColor()->low(color);
    }
  }
  else
    *machine.memory().cursor() = { 0, 0 };

  return 0;
}

int debugprint(lua_State* L)
{
  std::string text = lua_tostring(L, 1);
  std::cout << text << std::endl;

  return 0;
}

namespace sprites
{
  int fget(lua_State* L)
  {
    retro8::sprite_index_t index = lua_tonumber(L, 1);
    retro8::sprite_flags_t flags = *machine.memory().spriteFlagsFor(index);

    if (lua_gettop(L) == 2)
    {
      int index = lua_tonumber(L, 2);
      assert(index >= 0 && index <= 7);
      lua_pushboolean(L, (flags >> index) & 0x1 ? true : false);
    }
    else
      lua_pushnumber(L, *machine.memory().spriteFlagsFor(index));

    return 1;
  }

  int fset(lua_State* L)
  {
    retro8::sprite_index_t index = lua_tonumber(L, 1);
    retro8::sprite_flags_t* flags = machine.memory().spriteFlagsFor(index);

    if (lua_gettop(L) == 3)
    {
      int index = lua_tonumber(L, 2);
      bool value = lua_toboolean(L, 3);
      assert(index >= 0 && index <= 7);

      if (value)
        *flags = *flags | (1 << index);
      else
        *flags = *flags & ~(1 << index);
    }
    else
    {
      retro8::sprite_flags_t value = lua_tonumber(L, 2);
      *flags = value;
    }

    return 0;
  }

  int sspr(lua_State* L)
  {
    //TODO: implement

    return 0;
  }
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

  int max(lua_State* L)
  {
    assert(lua_isnumber(L, 1));
    assert(lua_isnumber(L, 2));

    real_t v1 = lua_tonumber(L, 1), v2 = lua_tonumber(L, 2);
    lua_pushnumber(L, std::max(v1, v2));

    return 1;
  }

  int mid(lua_State* L)
  {
    assert(lua_isnumber(L, 1));
    assert(lua_isnumber(L, 2));
    assert(lua_isnumber(L, 3));

    real_t a = lua_tonumber(L, 1);
    real_t b = lua_tonumber(L, 2);
    real_t c = lua_tonumber(L, 3);

    if ((a < b && b < c) || (c < b && b < a))
      lua_pushnumber(L, b);
    else if ((b < a && a < c) || (c < a && a < b))
      lua_pushnumber(L, a);
    else
      lua_pushnumber(L, c);

    return 1;
  }

  int abs(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t v = lua_tonumber(L, 1);
    lua_pushnumber(L, std::abs(v));

    return 1;
  }

  int sgn(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t v = lua_tonumber(L, 1);
    lua_pushnumber(L, v > 0 ? 1.0 : -1.0);

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

  int stat(lua_State* L)
  {
    //TODO: implement
    lua_pushinteger(L, 0);

    return 1;
  }

  int flip(lua_State* L)
  {
    machine.flip();

    return 0;
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
  lua_register(L, "clip", draw::clip);
  lua_register(L, "cls", cls);
  lua_register(L, "spr", spr);
  lua_register(L, "camera", camera);
  lua_register(L, "map", map);
  lua_register(L, "mget", mget);
  lua_register(L, "mset", mset);

  lua_register(L, "print", print);
  lua_register(L, "cursor", cursor);

  lua_register(L, "fset", sprites::fset);
  lua_register(L, "fget", sprites::fget);
  lua_register(L, "sspr", sprites::sspr);

  lua_register(L, "debug", debugprint);

  lua_register(L, "cos", math::cos);
  lua_register(L, "sin", math::sin);
  lua_register(L, "srand", math::srand);
  lua_register(L, "rnd", math::rnd);
  lua_register(L, "flr", math::flr);
  lua_register(L, "min", math::min);
  lua_register(L, "max", math::max);
  lua_register(L, "mid", math::mid);
  lua_register(L, "abs", math::abs);
  lua_register(L, "sgn", math::sgn);

  lua_register(L, "music", sound::music);
  lua_register(L, "sfx", sound::music);

  lua_register(L, "btn", platform::btn);
  lua_register(L, "btnp", platform::btnp);
  lua_register(L, "flip", platform::flip);
  lua_register(L, "stat", platform::stat);
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
