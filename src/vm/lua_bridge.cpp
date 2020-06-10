#include "lua_bridge.h"

#include "machine.h"
#include "lua/lua.hpp"
#include "gen/lua_api.h"

#include <functional>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

#pragma warning(push)
#pragma warning(disable: 4244)

using namespace lua;
using namespace retro8;

extern retro8::Machine machine;

using real_t = float;

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

int fillp(lua_State* L)
{
  //TODO: implement
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

  int c = lua_gettop(L) >= 5 ? lua_tonumber(L, 5) : machine.memory().penColor()->low();

  machine.rectfill(x0, y0, x1, y1, static_cast<color_t>(c));

  return 0;
}

int circ(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_gettop(L) >= 3 ? lua_tonumber(L, 3) : 4;
  int c = lua_gettop(L) >= 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circ(x, y, r, static_cast<color_t>(c));

  return 0;
}

int circfill(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  int r = lua_gettop(L) >= 3 ? lua_tonumber(L, 3) : 4;
  int c = lua_gettop(L) >= 4 ? lua_tonumber(L, 4) : machine.memory().penColor()->low();

  machine.circfill(x, y, r, color_t(c));

  return 0;
}

int cls(lua_State* L)
{
  int c = lua_gettop(L) == 1 ? lua_tonumber(L, -1) : 0;

  machine.cls(color_t(c));

  return 0;
}

int spr(lua_State* L)
{
  assert(lua_isnumber(L, 2) && lua_isnumber(L, 3));

  int idx = lua_tonumber(L, 1);
  int x = lua_tonumber(L, 2);
  int y = lua_tonumber(L, 3);

  if (lua_gettop(L) > 3)
  {
    assert(lua_gettop(L) >= 5);

    real_t w = lua_tonumber(L, 4);
    real_t h = lua_tonumber(L, 5);
    bool fx = false, fy = false;

    if (lua_gettop(L) >= 6)
      fx = lua_toboolean(L, 6);

    if (lua_gettop(L) >= 7)
      fy = lua_toboolean(L, 7);

    machine.spr(idx, x, y, w, h, fx, fy);
  }
  else
    /* optimized path */
    machine.spr(idx, x, y);

  return 0;
}

int sget(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);

  lua_pushnumber(L, machine.memory().spriteSheet(x, y)->get(x));

  return 1;
}

int sset(lua_State* L)
{
  int x = lua_tonumber(L, 1);
  int y = lua_tonumber(L, 2);
  color_t c = lua_gettop(L) >= 3 ? color_t((int)lua_tonumber(L, 3)) : machine.memory().penColor()->low();

  machine.memory().spriteSheet(x, y)->set(x, c);

  return 0;
}


int pal(lua_State* L)
{
  /* no arguments, reset palette */
  if (lua_gettop(L) == 0)
  {
    machine.memory().paletteAt(gfx::DRAW_PALETTE_INDEX)->reset();
    machine.memory().paletteAt(gfx::SCREEN_PALETTE_INDEX)->reset();
  }
  else
  {
    int c0 = lua_tonumber(L, 1);
    int c1 = lua_tonumber(L, 2);
    palette_index_t index = gfx::DRAW_PALETTE_INDEX;

    if (lua_gettop(L) == 3)
      index = lua_tonumber(L, 3);

    machine.pal(static_cast<color_t>(c0), static_cast<color_t>(c1), index);
  }
  return 0;
}

int palt(lua_State* L)
{
  /* no arguments, reset palette */
  if (lua_gettop(L) == 0)
  {
    machine.memory().paletteAt(gfx::DRAW_PALETTE_INDEX)->resetTransparency();
    machine.memory().paletteAt(gfx::SCREEN_PALETTE_INDEX)->resetTransparency();
  }
  else
  {
    color_t c = color_t(int(lua_tonumber(L, 1)));
    int f = lua_toboolean(L, 2);
    palette_index_t index = gfx::DRAW_PALETTE_INDEX;

    machine.memory().paletteAt(gfx::DRAW_PALETTE_INDEX)->transparent(c, f);
  }
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

      machine.memory().clipRect()->set(x0, y0, std::min(x0 + w, int32_t(gfx::SCREEN_WIDTH-1)), std::min(y0 + h, int32_t(gfx::SCREEN_HEIGHT-1)));
    }

    return 0;
  }
}



int camera(lua_State* L)
{
  int16_t cx = lua_gettop(L) >= 1 ? lua_tonumber(L, 1) : 0;
  int16_t cy = lua_gettop(L) == 2 ? lua_tonumber(L, 2) : 0;
  machine.memory().camera()->set(cx, cy);

  return 0;
}

int map(lua_State* L)
{
  coord_t cx = lua_tonumber(L, 1);
  coord_t cy = lua_tonumber(L, 2);
  coord_t x = lua_tonumber(L, 3);
  coord_t y = lua_tonumber(L, 4);
  amount_t cw = lua_gettop(L) >= 5 ? lua_tonumber(L, 5) : (gfx::SCREEN_WIDTH / gfx::SPRITE_WIDTH);
  amount_t ch = lua_gettop(L) >= 6 ? lua_tonumber(L, 6) : (gfx::SCREEN_HEIGHT / gfx::SPRITE_HEIGHT);
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

    retro8::coord_t x = cursor->x();
    retro8::coord_t y = cursor->y();
    retro8::color_t c = machine.memory().penColor()->low();
    machine.print(text, x, y, c);
    cursor->set(cursor->x(), cursor->y() + TEXT_LINE_HEIGHT); //TODO: check height
  }
  else if (lua_gettop(L) >= 3)
  {
    int x = lua_tonumber(L, 2);
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

namespace debug
{
  int debugprint(lua_State* L)
  {
    std::string text = lua_tostring(L, 1);
    std::cout << text << std::endl;

    return 0;
  }

  int breakpoint(lua_State* L)
  {
#if _WIN32
    __debugbreak();
#endif
    return 0;
  }
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
    coord_t sx = lua_tonumber(L, 1);
    coord_t sy = lua_tonumber(L, 2);
    coord_t sw = lua_tonumber(L, 3);
    coord_t sh = lua_tonumber(L, 4);
    coord_t dx = lua_tonumber(L, 5);
    coord_t dy = lua_tonumber(L, 6);
    coord_t dw = lua_to_or_default(L, number, 7, sw);
    coord_t dh = lua_to_or_default(L, number, 8, sh);
    bool flipX = lua_to_or_default(L, boolean, 8, false);
    bool flipY = lua_to_or_default(L, boolean, 8, false);

    machine.sspr(sx, sy, sw, sh, dx, dy, dw, dh, flipX, flipY);

    return 0;
  }
}

namespace math
{
  using real_t = float;
  static constexpr float PI = 3.14159265358979323846;

  int cos(lua_State* L)
  {
    if (lua_isnumber(L, 1))
    {
      real_t angle = lua_tonumber(L, 1);
      real_t value = std::cos(angle * 2 * PI);
      lua_pushnumber(L, value);
    }
    else
      lua_pushnumber(L, 0);

    return 1;
  }

  int sin(lua_State* L)
  {
    if (lua_isnumber(L, 1))
    {
      real_t angle = lua_tonumber(L, 1);
      real_t value = std::sin(-angle * 2 * PI);
      lua_pushnumber(L, value);
    }
    else
      lua_pushnumber(L, 0);

    return 1;
  }

  int atan2(lua_State* L)
  {
    assert(lua_isnumber(L, 1));
    real_t dx = lua_tonumber(L, 1);
    real_t dy = lua_tonumber(L, 2);
    real_t value = std::atan2(dx, dy) / (2 * PI) - 0.25;
    if (value < 0.0)
      value += 1.0;

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

#define FAIL_IF_NOT_NUMBER(i) do { if (!lua_isnumber(L, i)) { printf("Expected number but got %s\n", lua_typename(L, i)); assert(false); } } while (false)

  int rnd(lua_State* L)
  {
    real_t max = lua_gettop(L) >= 1 ? lua_tonumber(L, 1) : 1.0f;
    lua_pushnumber(L, (machine.state().rnd() / (float)machine.state().rnd.max()) * max);

    return 1;
  }

  int flr(lua_State* L)
  {
    real_t value = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
    lua_pushnumber(L, std::floor(value));
    return 1;
  }

  int ceil(lua_State* L)
  {
    real_t value = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
    lua_pushnumber(L, std::ceil(value));
    return 1;
  }


  int min(lua_State* L)
  {
    real_t v1 = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
    real_t v2 = 0;

    if (lua_gettop(L) == 2 && lua_isnumber(L, 2))
      v2 = lua_tonumber(L, 2);

    lua_pushnumber(L, std::min(v1, v2));

    return 1;
  }

  int max(lua_State* L)
  {
    real_t v1 = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
    real_t v2 = 0;

    if (lua_gettop(L) == 2 && lua_isnumber(L, 2))
    {
      FAIL_IF_NOT_NUMBER(2);
      v2 = lua_tonumber(L, 2);
    }

    lua_pushnumber(L, std::max(v1, v2));

    return 1;
  }

  int mid(lua_State* L)
  {
    real_t a = lua_tonumber(L, 1);
    real_t b = lua_tonumber(L, 2);
    real_t c = lua_gettop(L) >= 3 ? lua_tonumber(L, 3) : 0;

    if ((a <= b && b <= c) || (c <= b && b <= a))
      lua_pushnumber(L, b);
    else if ((b <= a && a <= c) || (c <= a && a <= b))
      lua_pushnumber(L, a);
    else
      lua_pushnumber(L, c);

    return 1;
  }

  int abs(lua_State* L)
  {
    if (lua_isnumber(L, 1))
    {
      real_t v = lua_tonumber(L, 1);
      lua_pushnumber(L, std::abs(v));
    }
    else
      lua_pushnumber(L, 0);

    return 1;
  }

  int sgn(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t v = lua_tonumber(L, 1);
    lua_pushnumber(L, v > 0 ? 1.0 : -1.0);

    return 1;
  }

  int sqrt(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    real_t v = lua_tonumber(L, 1);
    lua_pushnumber(L, sqrtf(v));

    return 1;
  }
}

#define EXPECT_TYPE(tn, idx) do { if (!lua_is ## tn(L, idx)) std::cout << "expected " # tn << " but got " << lua_typename(L, idx) << std::endl; assert(false);} while (false)

namespace bitwise
{
  using data_t = uint32_t;
  static constexpr size_t DATA_WIDTH = 32;

  template<typename F>
  int bitwise(lua_State* L)
  {
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
      data_t a = lua_tonumber(L, 1);
      data_t b = lua_tonumber(L, 2);

      lua_pushnumber(L, F()(a, b));
    }
    else
      lua_pushnumber(L, 0);


    return 1;
  }

  struct shift_left
  {
    data_t operator()(data_t v, data_t a) { return v << a; }
  };

  struct shift_right
  {
    data_t operator()(data_t v, data_t a) { return v >> a; }
  };

  struct rotate_left
  {
    data_t operator()(data_t v, data_t a) { return (v << a) | (v >> (DATA_WIDTH - a)); }
  };

  struct rotate_right
  {
    data_t operator()(data_t v, data_t a) { return (v >> a) | (v << (DATA_WIDTH - a)); }
  };

  inline int band(lua_State* L) { return bitwise<std::bit_and<data_t>>(L); }
  inline int bor(lua_State* L) { return bitwise<std::bit_or<data_t>>(L); }
  inline int bxor(lua_State* L) { return bitwise<std::bit_xor<data_t>>(L); }

  //TODO FIXME: fixme check implementation of logical and arithmetic shifts here

  inline int shl(lua_State* L) { return bitwise<shift_left>(L); }
  inline int shr(lua_State* L) { return bitwise<shift_right>(L); }
  
  inline int lshl(lua_State* L) { return bitwise<shift_left>(L); }
  inline int lshr(lua_State* L) { return bitwise<shift_right>(L); }
  
  inline int rotl(lua_State* L) { return bitwise<rotate_left>(L); }
  inline int rotr(lua_State* L) { return bitwise<rotate_right>(L); }


  int bnot(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    data_t a = lua_tonumber(L, 1);

    lua_pushnumber(L, std::bit_not<data_t>()(a));

    return 1;
  }
}

namespace sound
{
  int music(lua_State* L)
  {
    sfx::music_index_t index = lua_tonumber(L, 1);
    int32_t fadeMs = lua_to_or_default(L, number, 2, 1);
    int32_t mask = lua_to_or_default(L, number, 3, 0);

    machine.sound().music(index, fadeMs, mask);

    return 0;
  }

  int sfx(lua_State* L)
  {
    sfx::sound_index_t index = lua_tonumber(L, 1);
    sfx::channel_index_t channel = lua_to_or_default(L, number, 2, -1);
    int32_t start = lua_to_or_default(L, number, 3, 0);
    int32_t end = lua_to_or_default(L, number, 3, machine.memory().sound(index)->length());

    machine.sound().play(index, channel, start, end);

    return 0;
  }
}

namespace string
{
  int sub(lua_State* L)
  {
    const std::string v = lua_tostring(L, 1);
    size_t s = lua_tonumber(L, 2);
    size_t e = lua_to_or_default(L, number, 3, -1);

    size_t len = v.length();
    if (s < 0)
      s = len - s + 1;
    if (e < 0)
      e = len - e + 1;

    // TODO: intended behavior? picotetris calls it with swapped indices
    if (e < s || s > len)
      lua_pushstring(L, "");
    else
    {
      if (s == 0)
        s = 1;

      lua_pushstring(L, v.substr(s - 1, e - s + 1).c_str());
    }


    return 1;
  }

  int tostr(lua_State* L)
  {
    //TODO implement

    static char buffer[20];

    switch (lua_type(L, 1))
    {
    case LUA_TBOOLEAN: lua_pushstring(L, lua_toboolean(L, 1) ? "true" : "false"); break;
    case LUA_TNUMBER:
    {
      snprintf(buffer, 20, "% 4.4f", lua_tonumber(L, 1));
      lua_pushstring(L, buffer);
      break;
    }
    case LUA_TSTRING: lua_pushstring(L, lua_tostring(L, 1)); break;
    default: lua_pushstring(L, "foo");
    }

    return 1;
  }

  int tonum(lua_State* L)
  {
    //TODO implement
    const char* string = lua_tostring(L, 1);

    lua_pushnumber(L, 0);

    return 1;
  }
}

namespace platform
{
  int poke(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint8_t byte = lua_tonumber(L, 2);

    machine.memory().base()[addr] = byte;

    return 0;
  }

  int poke2(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint32_t value = lua_tonumber(L, 2);

    machine.memory().base()[addr] = value & 0xFF;
    machine.memory().base()[addr+1] = (value & 0xFF00) >> 8;

    return 0;
  }

  int poke4(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint32_t value = lua_tonumber(L, 2);

    machine.memory().base()[addr] = value & 0xFF;
    machine.memory().base()[addr + 1] = (value & 0xFF00) >> 8;
    machine.memory().base()[addr + 2] = (value & 0xFF0000) >> 16;
    machine.memory().base()[addr + 3] = (value & 0xFF000000) >> 24;

    return 0;
  }

  int peek(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint8_t value = machine.memory().base()[addr];

    lua_pushnumber(L, value);

    return 1;
  }

  int peek2(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint8_t low = machine.memory().base()[addr];
    uint8_t high = machine.memory().base()[addr+1];

    lua_pushnumber(L, (low | high << 8));

    return 1;
  }

  int peek4(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint8_t b1 = machine.memory().base()[addr];
    uint8_t b2 = machine.memory().base()[addr + 1];
    uint8_t b3 = machine.memory().base()[addr + 2];
    uint8_t b4 = machine.memory().base()[addr + 3];

    lua_pushnumber(L, b1 | (b2 << 8) | (b3 << 16) | (b4 << 24));

    return 1;
  }

  int memset(lua_State* L)
  {
    address_t addr = lua_tonumber(L, 1);
    uint8_t value = lua_tonumber(L, 2);
    int32_t length = lua_tonumber(L, 3);

    if (length > 0)
      std::memset(machine.memory().base() + addr, 0, length);

    return 0;
  }

  int memcpy(lua_State* L)
  {
    address_t dest = lua_tonumber(L, 1);
    address_t src = lua_tonumber(L, 2);
    int32_t length = lua_tonumber(L, 3);

    //TODO: optimize overlap case?
    if ((src + length < dest) || (dest + length < src))
      std::memcpy(machine.memory().base() + dest, machine.memory().base() + src, length);
    else
    {
      for (size_t i = 0; i < length; ++i)
        machine.memory().base()[dest + i] = machine.memory().base()[src + i];
    }

    return 0;
  }

  int reload(lua_State* L)
  {
    assert(lua_gettop(L) <= 3);
    
    address_t dest = lua_to_or_default(L, number, 1, 0);
    address_t src = lua_to_or_default(L, number, 1, 0);
    int32_t length = lua_to_or_default(L, number, 1, address::CART_DATA_LENGTH);
    
    std::memcpy(machine.memory().base() + dest, machine.memory().backup() + src, length);

    return 0;
  }

  int btn(lua_State* L)
  {
    index_t index = lua_gettop(L) >= 2 ? lua_tonumber(L, 2) : 0;
    if (index >= PLAYER_COUNT) index = 0;
 
    /* we're asking for a specific button*/
    if (lua_gettop(L) >= 1)
    {
      using bt_t = retro8::button_t;
      static constexpr std::array<bt_t, 6> buttons = { bt_t::LEFT, bt_t::RIGHT, bt_t::UP, bt_t::DOWN, bt_t::ACTION1, bt_t::ACTION2 };
      size_t bindex = lua_tonumber(L, 1);

      if (bindex < buttons.size())
        lua_pushboolean(L, machine.state().buttons[index].isSet(buttons[bindex]));
      else
        lua_pushboolean(L, false);

    }
    /* push whole bitmask*/
    else
    {
      lua_pushnumber(L, machine.state().buttons[index].value);
    }

    //TODO: finish for player 2?
    return 1;
  }

  int btnp(lua_State* L)
  {
    const index_t index = lua_gettop(L) >= 2 ? lua_tonumber(L, 2) : 0;

    //TODO: check behavior

    /* we're asking for a specific button*/
    if (lua_gettop(L) >= 1)
    {
      using bt_t = retro8::button_t;
      static constexpr std::array<bt_t, 6> buttons = { bt_t::LEFT, bt_t::RIGHT, bt_t::UP, bt_t::DOWN, bt_t::ACTION1, bt_t::ACTION2 };
      size_t bindex = lua_tonumber(L, 1);
      lua_pushboolean(L, machine.state().previousButtons[index].isSet(buttons[bindex]));
    }
    /* push whole bitmask*/
    else
    {
      lua_pushnumber(L, machine.state().previousButtons[index].value);
    }

    //TODO: finish for player?
    return 1;
  }

  int stat(lua_State* L)
  {
    //TODO: implement

    enum class Stat { FRAME_RATE = 7 };
    Stat s = static_cast<Stat>((int)lua_tonumber(L, -1));


    switch (s)
    {
    case Stat::FRAME_RATE: lua_pushnumber(L, machine.code().require60fps() ? 60 : 30); break;
    default: lua_pushnumber(L, 0);

    }

    return 1;
  }

  int cartdata(lua_State* L)
  {
    //TODO: implement
    return 0;
  }

  int dset(lua_State* L)
  {
    index_t idx = lua_tonumber(L, 1);
    integral_t value = lua_tonumber(L, 2);

    *machine.memory().cartData(idx) = value;
    return 0;
  }

  int dget(lua_State* L)
  {
    index_t idx = lua_tonumber(L, 1);

    lua_pushnumber(L, *machine.memory().cartData(idx));

    return 1;
  }

  int flip(lua_State* L)
  {
    //TODO: this call should syncronize to 30fps, at the moment it just
    // returns producing a lot of flips in non synchronized code (eg. _init() busy loop)
    //TODO: flip is handled by backend so we should find a way to set the callback that should be called

    return 0;
  }

  int extcmd(lua_State* L)
  {
    //TODO: implement
    return 0;
  }

  int menuitem(lua_State* L)
  {
    //TODO: implement
    return 0;
  }

  int time(lua_State* L)
  {
    lua_pushnumber(L, Platform::getTicks() / 1000.0f);
    return 1;
  }

  int printh(lua_State* L)
  {
    //TODO: finish implementing additional parameters
    
    std::string text = lua_tostring(L, 1);
    std::cout << text << std::endl;

    return 0;
  }
}

#pragma warning(pop)

void lua::registerFunctions(lua_State* L)
{
  lua_register(L, "pset", pset);
  lua_register(L, "pget", pget);
  lua_register(L, "pal", pal);
  lua_register(L, "palt", palt);
  lua_register(L, "color", color);
  lua_register(L, "line", line);
  lua_register(L, "fillp", fillp);
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
  lua_register(L, "sget", sget);
  lua_register(L, "sset", sset);

  lua_register(L, "print", print);
  lua_register(L, "cursor", cursor);

  lua_register(L, "fset", sprites::fset);
  lua_register(L, "fget", sprites::fget);
  lua_register(L, "sspr", sprites::sspr);

  lua_register(L, "__debugprint", debug::debugprint);
  lua_register(L, "__breakpoint", debug::breakpoint);

  lua_register(L, "cos", math::cos);
  lua_register(L, "sin", math::sin);
  lua_register(L, "atan2", math::atan2);
  lua_register(L, "srand", math::srand);
  lua_register(L, "rnd", math::rnd);
  lua_register(L, "flr", math::flr);
  lua_register(L, "ceil", math::ceil);
  lua_register(L, "min", math::min);
  lua_register(L, "max", math::max);
  lua_register(L, "mid", math::mid);
  lua_register(L, "abs", math::abs);
  lua_register(L, "sgn", math::sgn);
  lua_register(L, "sqrt", math::sqrt);

  lua_register(L, "band", bitwise::band);
  lua_register(L, "bor", bitwise::bor);
  lua_register(L, "bxor", bitwise::bxor);
  lua_register(L, "bnot", bitwise::bnot);
  lua_register(L, "shl", bitwise::shl);
  lua_register(L, "shr", bitwise::shr);
  lua_register(L, "lshl", bitwise::lshl);
  lua_register(L, "lshr", bitwise::lshr);
  lua_register(L, "rotl", bitwise::rotl);
  lua_register(L, "rotr", bitwise::rotr);

  lua_register(L, "music", ::sound::music);
  lua_register(L, "sfx", ::sound::sfx);

  lua_register(L, "sub", string::sub);
  lua_register(L, "tostr", string::tostr);
  lua_register(L, "tonum", string::tonum);

  lua_register(L, "btn", platform::btn);
  lua_register(L, "btnp", platform::btnp);
  lua_register(L, "time", platform::time);
  lua_register(L, "t", platform::time);
  lua_register(L, "extcmd", platform::extcmd);
  lua_register(L, "menuitem", platform::menuitem);
  lua_register(L, "stat", platform::stat);
  lua_register(L, "cartdata", platform::cartdata);
  lua_register(L, "dset", platform::dset);
  lua_register(L, "dget", platform::dget);
  lua_register(L, "poke", platform::poke);
  lua_register(L, "peek", platform::peek);
  lua_register(L, "poke2", platform::poke2);
  lua_register(L, "peek2", platform::peek2);
  lua_register(L, "poke4", platform::poke4);
  lua_register(L, "peek4", platform::peek4);
  lua_register(L, "memset", platform::memset);
  lua_register(L, "memcpy", platform::memcpy);
  lua_register(L, "reload", platform::reload);
  lua_register(L, "printh", platform::printh);

  lua_register(L, "flip", platform::flip);
}

Code::~Code()
{
  if (L)
    lua_close(L);
}

void Code::loadAPI()
{
  if (!L)
  {
    L = luaL_newstate();
  }

  luaL_openlibs(L);

  /*std::ifstream apiFile("api.lua");
  std::string api((std::istreambuf_iterator<char>(apiFile)), std::istreambuf_iterator<char>());*/

  LOGD("Loading extended PICO-8 Api");

  if (luaL_dostring(L, lua_api_string))
    printError("api.lua loading");
}

void Code::printError(const char* where)
{
  //for (int i = 1; i < lua_gettop(L); ++i)
  {
    std::cout << "Error on " << where << std::endl;

    //luaL_traceback(L, L, NULL, 1);
    //printf("%s\n", lua_tostring(L, -1));

    if (lua_isstring(L, -1))
    {
      const char* message = lua_tostring(L, -1);
      std::cout << message << std::endl;
    }
  }
  getchar();
}

void Code::initFromSource(const std::string& code)
{
  if (!L)
    L = luaL_newstate();

  registerFunctions(L);



  if (luaL_loadstring(L, code.c_str()))
    printError("luaL_loadString");

  int error = lua_pcall(L, 0, 0, 0);

  if (error)
    printError("lua_pcall on init");


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

void Code::callFunction(const char* name, int ret)
{
  lua_getglobal(L, name);
  int error = lua_pcall(L, 0, ret, 0);

  if (error)
    printError(name);
}

void Code::update()
{
  if (_update60)
    callFunction("_update60");
  else if (_update)
    callFunction("_update");
}

void Code::draw()
{
  if (_draw)
    callFunction("_draw");
}

void Code::init()
{
  if (_init)
    callFunction("_init");
}
