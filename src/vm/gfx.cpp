#include "gfx.h"

#include "gen/pico_font.h"


using namespace retro8;
using namespace retro8::gfx;

void Font::load()
{
  // 128 x 80 bitmap (1 bit per pixel)

  constexpr size_t BITS_PER_BYTE = 8;
  constexpr size_t BYTES_PER_SPRITE = SPRITE_WIDTH * SPRITE_HEIGHT / 8;
  constexpr size_t TOTAL_BYTES = FONT_GLYPHS_COLUMNS * FONT_GLYPHS_ROWS * BYTES_PER_SPRITE;
  constexpr size_t BYTES_PER_ROW = FONT_GLYPHS_COLUMNS * BYTES_PER_SPRITE;
  
  static_assert(TOTAL_BYTES == sizeof(font_map) / sizeof(font_map[0]), "Must be equal");
  static_assert(BYTES_PER_ROW == 128, "");
  
  for (size_t i = 0; i < TOTAL_BYTES; ++i)
  {
    const size_t row = i / BYTES_PER_ROW;
    const size_t col = i % FONT_GLYPHS_COLUMNS;
    const size_t index = row * FONT_GLYPHS_COLUMNS + col;
    const size_t y = (i - (row * BYTES_PER_ROW)) / FONT_GLYPHS_COLUMNS;

    const auto byte = font_map[i];
    sequential_sprite_t& glyph = glyphs[index];
    
    for (size_t j = 0; j < BITS_PER_BYTE; ++j)
    {
      const bool s = (byte & (1 << (BITS_PER_BYTE - j - 1))) != 0;
      const size_t x = ((i * BITS_PER_BYTE) + j) % 8;

      glyph.set(x, y, s ? color_t::WHITE : color_t::BLACK);
    }
  }
}

void Font::load(const uint8_t* data)
{
  //assert(surface->w == SPRITE_WIDTH * FONT_GLYPHS_COLUMNS && surface->h == SPRITE_HEIGHT * FONT_GLYPHS_ROWS);

  constexpr size_t pitch = SPRITE_WIDTH * FONT_GLYPHS_COLUMNS;

  for (size_t gy = 0; gy < FONT_GLYPHS_ROWS; ++gy)
    for (size_t gx = 0; gx < FONT_GLYPHS_COLUMNS; ++gx)
    {
      sequential_sprite_t& glyph = glyphs[gy*FONT_GLYPHS_COLUMNS + gx];

      for (size_t sy = 0; sy < SPRITE_HEIGHT; ++sy)
        for (size_t sx = 0; sx < SPRITE_WIDTH; ++sx)
        {
          size_t bx = gx * SPRITE_WIDTH;
          size_t by = gy * SPRITE_HEIGHT;
          size_t index = (by + sy) * pitch + bx + sx;

          glyph.set(sx, sy, data[index] ? color_t::WHITE : color_t::BLACK);
        }
    }
}
