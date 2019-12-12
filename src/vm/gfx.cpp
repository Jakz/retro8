#include "gfx.h"

using namespace retro8;
using namespace retro8::gfx;

std::array<uint32_t, COLOR_COUNT> ColorTable::table;
void ColorTable::init(SDL_PixelFormat* format)
{
  static constexpr std::array<SDL_Color, COLOR_COUNT> colors = {
    SDL_Color{  0,   0,   0}, SDL_Color{ 29,  43,  83}, SDL_Color{126,  37,  83}, SDL_Color{  0, 135,  81},
    SDL_Color{171,  82,  54}, SDL_Color{ 95,  87,  79}, SDL_Color{194, 195, 199}, SDL_Color{255, 241, 232},
    SDL_Color{255,   0,  77}, SDL_Color{255, 163,   0}, SDL_Color{255, 236,  39}, SDL_Color{  0, 228,  54},
    SDL_Color{ 41, 173, 255}, SDL_Color{131, 118, 156}, SDL_Color{255, 119, 168}, SDL_Color{255, 204, 170}
  };

  for (size_t i = 0; i < COLOR_COUNT; ++i)
    table[i] = SDL_MapRGB(format, colors[i].r, colors[i].g, colors[i].b);
}

void Font::load(SDL_Surface* surface)
{
  assert(surface->w == SPRITE_WIDTH * FONT_GLYPHS_COLUMNS && surface->h == SPRITE_HEIGHT * FONT_GLYPHS_ROWS);

  for (size_t gy = 0; gy < FONT_GLYPHS_ROWS; ++gy)
    for (size_t gx = 0; gx < FONT_GLYPHS_COLUMNS; ++gx)
    {
      sequential_sprite_t& glyph = glyphs[gy*FONT_GLYPHS_COLUMNS + gx];

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