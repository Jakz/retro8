#pragma once

#include "common.h"
#include "defines.h"
#include "gfx.h"
#include "lua_bridge.h"

#include <SDL.h>
#include <array>
#include <random>

namespace retro8
{
  class State
  {
  public:
    std::mt19937 rnd;
    point_t lastLineEnd;
    bit_mask<button_t> buttons;
    bit_mask<button_t> previousButtons;
  };

  namespace address
  {
    static constexpr address_t SPRITE_SHEET = 0x0000;
    static constexpr address_t SPRITE_FLAGS = 0x3000;

    static constexpr address_t PALETTES = 0x5f00;
    static constexpr address_t CLIP_RECT = 0x5f20;
    static constexpr address_t PEN_COLOR = 0x5f25;
    static constexpr address_t CURSOR = 0x5f26;
    static constexpr address_t CAMERA = 0x5f28;

    static constexpr address_t SCREEN_DATA = 0x6000;

    static constexpr address_t TILE_MAP_LOWER = 0x1000;
    static constexpr address_t TILE_MAP_HIGH = 0x2000;
  };

  class Memory
  {
  private:
    uint8_t memory[1024 * 32];



    static constexpr size_t BYTES_PER_SCREEN_ROW = 128;
    static constexpr size_t BYTES_PER_PALETTE = sizeof(retro8::gfx::palette_t);
    static constexpr size_t BYTES_PER_SPRITE = sizeof(retro8::gfx::sprite_t);

    static constexpr size_t ROWS_PER_TILE_MAP_HALF = 32;


  public:
    Memory()
    {
      paletteAt(gfx::DRAW_PALETTE_INDEX)->reset();
      paletteAt(gfx::SCREEN_PALETTE_INDEX)->reset();
      clipRect()->reset();
      *cursor() = { 0, 0 };
    }

    gfx::color_byte_t* penColor() { return reinterpret_cast<gfx::color_byte_t*>(&memory[address::PEN_COLOR]); }
    gfx::cursor_t* cursor() { return as<gfx::cursor_t>(address::CURSOR); }
    gfx::camera_t* camera() { return as<gfx::camera_t>(address::CAMERA); }
    gfx::clip_rect_t* clipRect() { return as<gfx::clip_rect_t>(address::CLIP_RECT); }
    

    gfx::color_byte_t* screenData() { return reinterpret_cast<gfx::color_byte_t*>(&memory[address::SCREEN_DATA]); }
    gfx::color_byte_t* screenData(coord_t x, coord_t y) { return screenData() + (y * BYTES_PER_SCREEN_ROW + x) / 2; }

    sprite_flags_t* spriteFlagsFor(sprite_index_t index)
    {
      return as<sprite_flags_t>(address::SPRITE_FLAGS + index);
    }

    sprite_index_t* spriteInTileMap(coord_t x, coord_t y)
    {
      static_assert(sizeof(sprite_index_t) == 1, "sprite_index_t must be 1 byte");

      if (y < ROWS_PER_TILE_MAP_HALF)
        return as<sprite_index_t>(address::TILE_MAP_HIGH) + x + y * gfx::TILE_MAP_WIDTH * sizeof(sprite_index_t);
      else
        return as<sprite_index_t>(address::TILE_MAP_HIGH) + x + (y - ROWS_PER_TILE_MAP_HALF) * gfx::TILE_MAP_WIDTH * sizeof(sprite_index_t);
    }

    gfx::sprite_t* spriteAt(sprite_index_t index) { 
      return reinterpret_cast<gfx::sprite_t*>(&memory[address::SPRITE_SHEET 
        + (index % gfx::SPRITES_PER_SPRITE_SHEET_ROW) * gfx::SPRITE_BYTES_PER_SPRITE_ROW]
        + (index / gfx::SPRITES_PER_SPRITE_SHEET_ROW) * gfx::SPRITE_SHEET_WIDTH_IN_BYTES * gfx::SPRITE_HEIGHT
        ); }
    gfx::palette_t* paletteAt(size_t index) { return reinterpret_cast<gfx::palette_t*>(&memory[address::PALETTES + index * BYTES_PER_PALETTE]); }

    template<typename T> T* as(address_t addr) { return reinterpret_cast<T*>(&memory[addr]); }
  };

  class Machine
  {
  private:
    State _state;
    Memory _memory;
    gfx::Font _font;
    lua::Code _code;
    SDL_Surface* _output;

  private:
    void circHelper(coord_t xc, coord_t yc, coord_t x, coord_t y, color_t col);

  public:
    Machine()
    {
    }

    void init(SDL_Surface* output) { _output = output; }

    void flip();

    void color(color_t color);

    void cls(color_t color);

    void pset(coord_t x, coord_t y, color_t color);
    color_t pget(coord_t x, coord_t y);

    void line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void rect(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void rectfill(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void circ(coord_t x, coord_t y, amount_t r, color_t color);
    void circfill(coord_t x, coord_t y, amount_t r, color_t color);

    void pal(color_t c0, color_t c1);

    void map(coord_t cx, coord_t cy, coord_t x, coord_t y, amount_t cw, amount_t ch, sprite_flags_t layer);
    void spr(index_t idx, coord_t x, coord_t y);
    void print(const std::string& string, coord_t x, coord_t y, color_t color);

    State& state() { return _state; }
    Memory& memory() { return _memory; }
    gfx::Font& font() { return _font; }
    lua::Code& code() { return _code; }
  };
}