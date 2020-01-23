#pragma once

#include "common.h"
#include "defines.h"
#include "gfx.h"
#include "sound.h"
#include "lua_bridge.h"
#include "memory.h"

#include <array>
#include <random>

namespace retro8
{
  class State
  {
  public:
    std::mt19937 rnd;
    point_t lastLineEnd;
    std::array<bit_mask<button_t>, PLAYER_COUNT> buttons;
    std::array<bit_mask<button_t>, PLAYER_COUNT> previousButtons;
  };

  class Machine
  {
  private:
    State _state;
    Memory _memory;
    sfx::APU _sound;
    gfx::Font _font;
    lua::Code _code;

  private:
    void circHelper(coord_t xc, coord_t yc, coord_t x, coord_t y, color_t col);
    void circFillHelper(coord_t xc, coord_t yc, coord_t x, coord_t y, color_t col);


  public:
    Machine() : _sound(_memory)
    {
    }

    Machine(const Machine&) = delete;
    Machine& operator=(const Machine&) = delete;

    void color(color_t color);

    void cls(color_t color);

    void pset(coord_t x, coord_t y, color_t color);
    color_t pget(coord_t x, coord_t y);

    void line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void rect(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void rectfill(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void circ(coord_t x, coord_t y, amount_t r, color_t color);
    void circfill(coord_t x, coord_t y, amount_t r, color_t color);

    void pal(color_t c0, color_t c1, palette_index_t index);

    void map(coord_t cx, coord_t cy, coord_t x, coord_t y, amount_t cw, amount_t ch, sprite_flags_t layer);
    void spr(index_t idx, coord_t x, coord_t y);
    void spr(index_t idx, coord_t x, coord_t y, float w, float h, bool flipX, bool flipY);
    void sspr(coord_t sx, coord_t sy, coord_t sw, coord_t sh, coord_t dx, coord_t dy, coord_t dw, coord_t dh, bool flipX, bool flipY);

    void print(const std::string& string, coord_t x, coord_t y, color_t color);

    State& state() { return _state; }
    Memory& memory() { return _memory; }
    gfx::Font& font() { return _font; }
    lua::Code& code() { return _code; }
    sfx::APU& sound() { return _sound; }
  };
}
