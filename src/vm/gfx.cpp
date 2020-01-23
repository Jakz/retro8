#include "gfx.h"

using namespace retro8;
using namespace retro8::gfx;

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
