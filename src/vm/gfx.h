#pragma once

#include "defines.h"

#include <array>
#include <cassert>

namespace retro8
{
  namespace gfx
  {
    static constexpr size_t PIXEL_TO_BYTE_RATIO = 2;

    static constexpr size_t SPRITE_WIDTH = 8;
    static constexpr size_t SPRITE_HEIGHT = 8;

    static constexpr size_t GLYPH_WIDTH = 4;
    static constexpr size_t GLYPH_HEIGHT = 6;

    static constexpr size_t SPRITE_BYTES_PER_SPRITE_ROW = SPRITE_WIDTH / PIXEL_TO_BYTE_RATIO;
    static constexpr size_t PALETTE_SIZE = 16;

    static constexpr size_t SCREEN_WIDTH = 128;
    static constexpr size_t SCREEN_HEIGHT = 128;
    static constexpr size_t SCREEN_PITCH = SCREEN_WIDTH / PIXEL_TO_BYTE_RATIO;;
    static constexpr size_t BYTES_PER_SCREEN = SCREEN_WIDTH * SCREEN_HEIGHT / PIXEL_TO_BYTE_RATIO;

    static constexpr size_t TILE_MAP_WIDTH = 128;
    static constexpr size_t TILE_MAP_HEIGHT = 64;


    static constexpr size_t SPRITE_SHEET_WIDTH = 128;
    static constexpr size_t SPRITES_PER_SPRITE_SHEET_ROW = 16;
    static constexpr size_t SPRITE_SHEET_PITCH = SPRITE_SHEET_WIDTH / PIXEL_TO_BYTE_RATIO;
    static constexpr size_t SPRITE_SHEET_HEIGHT = 128;
    static constexpr size_t SPRITE_COUNT = (SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT);

    static constexpr size_t FONT_GLYPHS_COLUMNS = 16;
    static constexpr size_t FONT_GLYPHS_ROWS = 10;

    static constexpr size_t DRAW_PALETTE_INDEX = 0;
    static constexpr size_t SCREEN_PALETTE_INDEX = 1;

    static constexpr size_t COLOR_COUNT = 16;
    
    //TODO: optimize by generating it the same format as the destination surface

    struct ColorTable
    {
    public:
      using pixel_t = uint32_t;

    private:
      std::array<pixel_t, COLOR_COUNT> table;

    public:
      template<typename B>
      void init(const B& mapper)
      {
        struct rgb_color_t { uint8_t r, g, b; };

        constexpr std::array<rgb_color_t, COLOR_COUNT> colors = { {
          {  0,   0,   0}, { 29,  43,  83}, {126,  37,  83}, {  0, 135,  81},
          {171,  82,  54}, { 95,  87,  79}, {194, 195, 199}, {255, 241, 232},
          {255,   0,  77}, {255, 163,   0}, {255, 236,  39}, {  0, 228,  54},
          { 41, 173, 255}, {131, 118, 156}, {255, 119, 168}, {255, 204, 170}
        } };

        for (size_t i = 0; i < COLOR_COUNT; ++i)
          table[i] = mapper(colors[i].r, colors[i].g, colors[i].b);
      }
      pixel_t get(color_t c) const { return table[c]; }
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
    public:
      color_t get(coord_t x, coord_t y) const { return byteAt(x, y).get(x); }
      void set(coord_t x, coord_t y, color_t color) { byteAt(x, y).set(x, color); }
      inline const color_byte_t& byteAt(coord_t x, coord_t y) const { return static_cast<const color_byte_t&>(((sprite_t*)this)->byteAt(x, y)); }
      inline color_byte_t& byteAt(coord_t x, coord_t y) { return reinterpret_cast<color_byte_t*>(this)[y * SPRITE_SHEET_PITCH + x / 2]; }
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
      std::array<uint8_t, COLOR_COUNT> colors;

    public:
      void reset()
      {
        for (size_t i = 0; i < COLOR_COUNT; ++i)
          colors[i] = uint8_t(i);
        transparent(color_t::BLACK, true);
      }

      void resetTransparency()
      {
        transparent(color_t::BLACK, true);

        for (size_t i = 1; i < COLOR_COUNT; ++i)
          transparent(color_t(i), false);
      }

      //TODO: %16 to make it wrap around, is it intended behavior? mandel.
      color_t get(color_t i) const { return color_t(colors[i] & 0x0F); }
      void set(color_t i, color_t color) { colors[i] = color | (colors[i] & 0x10); }
      color_t operator[](color_t i) { return get(i); }
      bool transparent(color_t i) const { return (colors[i] & 0x10) != 0; }
      void transparent(color_t i, bool f) { colors[i] = f ? (colors[i] | 0x10) : (colors[i] & 0x0f); }
    };

    struct clip_rect_t
    {
      uint8_t x0;
      uint8_t y0;
      uint8_t x1;
      uint8_t y1;

      void reset() { x0 = y0 = 0; x1 = SCREEN_WIDTH - 1; y1 = SCREEN_HEIGHT - 1; }
      void set(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye) { x0 = xs; y0 = ys; x1 = xe; y1 = ye; }
    };

    struct cursor_t
    {
      uint8_t _x, _y;

      void reset() { _x = _y = 0; }
      void set(uint8_t x, uint8_t y) { _x = x; _y = y; }
      uint8_t x() const { return _x; }
      uint8_t y() const { return _y; }
    };

    struct camera_t
    {

      int16_t _x;
      int16_t _y;

      int16_t x() const { return _x; }
      int16_t y() const { return _y; }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      //TODO: raw memory addresses expect these to be little endian, FIX!
      void set(int16_t x, int16_t y) { _x = x; _y = y; }
#else
      void set(int16_t x, int16_t y) { _x = x; _y = y; }
#endif

    };

    class Font
    {
      sequential_sprite_t glyphs[FONT_GLYPHS_ROWS*FONT_GLYPHS_COLUMNS];

    public:
      Font() { }
      inline const sequential_sprite_t* glyph(char c) const { return c < 128 ? &glyphs[c] : nullptr; }
      inline const sequential_sprite_t* specialGlyph(size_t i) const { return &glyphs[128+i]; }

      void load();

      /* this assumes a 1 byte per pixel bmp */
      void load(const uint8_t* data);
    };

  };


}