#include "gfx.h"

using namespace retro8;
using namespace retro8::gfx;

void Font::load(SDL_Surface* surface)
{
  assert(surface->w == SPRITE_WIDTH * FONT_GLYPHS_COLUMNS && surface->h == SPRITE_HEIGHT * FONT_GLYPHS_ROWS);

  for (size_t gy = 0; gy < FONT_GLYPHS_ROWS; ++gy)
    for (size_t gx = 0; gx < FONT_GLYPHS_COLUMNS; ++gx)
    {
      sprite_t& glyph = glyphs[gy*FONT_GLYPHS_COLUMNS + gx];

      for (size_t sy = 0; sy < SPRITE_HEIGHT; ++sy)
        for (size_t sx = 0; sx < SPRITE_WIDTH; ++sx)
        {
          size_t bx = gx * SPRITE_WIDTH;
          size_t by = gy * SPRITE_HEIGHT;
          size_t index = (by + sy) * surface->w + bx + sx;

          glyph.set(sx, sy, static_cast<const uint8_t*>(surface->pixels)[index] ? color_t::WHITE : color_t::BLACK);
        }
    }
}