#pragma once

#include "defines.h"

#include <SDL.h>
#include <array>
#include <cassert>

namespace retro8
{
  namespace gfx
  {
    //TODO: optimize by generating it the same format as the destination surface
    static constexpr std::array<SDL_Color, 16> ColorTable = {
      SDL_Color{  0,   0,   0}, SDL_Color{ 29,  43,  83}, SDL_Color{126,  37,  83}, SDL_Color{  0, 135,  81},
      SDL_Color{171,  82,  54}, SDL_Color{ 95,  87,  79}, SDL_Color{194, 195, 199}, SDL_Color{255, 241, 232},
      SDL_Color{255,   0,  77}, SDL_Color{255, 163,   0}, SDL_Color{255, 236,  39}, SDL_Color{  0, 228,  54},
      SDL_Color{ 41, 173, 255}, SDL_Color{131, 118, 156}, SDL_Color{255, 119, 168}, SDL_Color{255, 204, 170}
    };
    
    static color_t colorForRGB(uint32_t color)
    {
      switch (color & 0x00ffffff)
      {
        case 0x000000: return color_t::BLACK;
        case 0x1D2B53: return color_t::DARK_BLUE;
        case 0x7E2553: return color_t::DARK_PURPLE;
        case 0x008751: return color_t::DARK_GREEN;
        case 0xAB5236: return color_t::BROWN;
        case 0x5F574F: return color_t::DARK_GREY;
        case 0xC2C3C7: return color_t::LIGHT_GREY;
        case 0xFFF1E8: return color_t::WHITE;
        case 0xFF004D: return color_t::RED;
        case 0xFFA300: return color_t::ORANGE;
        case 0xFFEC27: return color_t::YELLOW;
        case 0x00E436: return color_t::GREEN;
        case 0x29ADFF: return color_t::BLUE;
        case 0x83769C: return color_t::INDIGO;
        case 0xFF77A8: return color_t::PINK;
        case 0xFFCCAA: return color_t::PEACH;
        default: assert(false);
      }
    }

    static constexpr size_t PIXEL_TO_BYTE_RATIO = 2;

    static constexpr size_t SPRITE_WIDTH = 8;
    static constexpr size_t SPRITE_HEIGHT = 8;
    
    static constexpr size_t GLYPH_WIDTH = 4;
    static constexpr size_t GLYPH_HEIGHT = 6;

    static constexpr size_t SPRITE_BYTES_PER_SPRITE_ROW = SPRITE_WIDTH / PIXEL_TO_BYTE_RATIO;
    static constexpr size_t PALETTE_SIZE = 16;

    static constexpr size_t SCREEN_WIDTH = 128;
    static constexpr size_t SCREEN_HEIGHT = 128;
    static constexpr size_t BYTES_PER_SCREEN = SCREEN_WIDTH * SCREEN_HEIGHT / PIXEL_TO_BYTE_RATIO;

    static constexpr size_t TILE_MAP_WIDTH = 128;
    static constexpr size_t TILE_MAP_HEIGHT = 64;


    static constexpr size_t SPRITE_SHEET_WIDTH = 128;
    static constexpr size_t SPRITES_PER_SPRITE_SHEET_ROW = 16;
    static constexpr size_t SPRITE_SHEET_WIDTH_IN_BYTES = SPRITE_SHEET_WIDTH / PIXEL_TO_BYTE_RATIO;
    static constexpr size_t SPRITE_SHEET_HEIGHT = 128;

    static constexpr size_t FONT_GLYPHS_COLUMNS = 16;
    static constexpr size_t FONT_GLYPHS_ROWS = 10;

    static constexpr size_t DRAW_PALETTE_INDEX = 0;
    static constexpr size_t SCREEN_PALETTE_INDEX = 1;

    struct color_byte_t
    {
      uint8_t value;

    public:
      color_byte_t() = default;
      color_byte_t(color_t low, color_t high) : value(low | (high << 4)) { }
      inline color_t low() const { return static_cast<color_t>(value & 0x0F); }
      inline color_t high() const { return static_cast<color_t>((value >> 4) & 0x0F); }
      inline void low(color_t color) { value = ((value & 0xf0) | color); }
      inline void high(color_t color) { value = ((value & 0x0f) | color << 4); }
      inline color_t get(coord_t mod) const { return (mod % 2) == 0 ? low() : high(); }
      inline void set(coord_t mod, color_t color) { value = (mod % 2) == 0 ? ((value & 0xf0) | color) : ((value & 0x0f) | color << 4); }
      inline void setBoth(color_t low, color_t high) { value = low | high << 4; }
    };

    class sprite_t
    {      
      inline const color_byte_t& byteAt(coord_t x, coord_t y) const { return static_cast<const color_byte_t&>(((sprite_t*)this)->byteAt(x, y)); }
      inline color_byte_t& byteAt(coord_t x, coord_t y) { return reinterpret_cast<color_byte_t*>(this)[y * SPRITE_SHEET_WIDTH_IN_BYTES + x / 2]; }

    public:
      color_t get(coord_t x, coord_t y) const { return byteAt(x, y).get(x); }
      void set(coord_t x, coord_t y, color_t color) { byteAt(x, y).set(x, color); }
    };

    class sequential_sprite_t
    {
      color_byte_t data[32];

      inline const color_byte_t& byteAt(coord_t x, coord_t y) const { return static_cast<const color_byte_t&>(((sequential_sprite_t*)this)->byteAt(x, y)); }
      inline color_byte_t& byteAt(coord_t x, coord_t y) { return data[y * SPRITE_BYTES_PER_SPRITE_ROW + x / 2]; }

    public:
      color_t get(coord_t x, coord_t y) const { return byteAt(x, y).get(x); }
      void set(coord_t x, coord_t y, color_t color) { byteAt(x, y).set(x, color); }
    };

    class palette_t
    {
      std::array<color_t, 16> colors;

    public:
      void reset()
      {
        for (size_t i = 0; i < 16; ++i)
          colors[i] = (color_t)i;
      }

      //TODO: %16 to make it wrap around, is it intended behavior? mandel.
      color_t get(color_t i) const { return colors[i%16]; }
      color_t set(color_t i, color_t color) { return colors[i] = color; }
      color_t& operator[](color_t i) { return colors[i]; }
    };

    class Font
    {
      sequential_sprite_t glyphs[FONT_GLYPHS_ROWS*FONT_GLYPHS_COLUMNS];

    public:
      Font() { }
      inline const sequential_sprite_t* glyph(char c) const { return &glyphs[c]; }

      void load(SDL_Surface* surface);
    };

  };


}