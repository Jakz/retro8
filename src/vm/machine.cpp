#include "machine.h"

using namespace retro8;

void Machine::flip()
{
  auto* data = _memory.screenData();
  auto* screenPalette = _memory.paletteAt(gfx::SCREEN_PALETTE_INDEX);
  auto* dest = static_cast<uint32_t*>(_surface->pixels);

  for (size_t i = 0; i < gfx::BYTES_PER_SCREEN; ++i)
  {
    const gfx::color_byte_t* pixels = data + i;
    
    const auto& rc1 = gfx::ColorTable[screenPalette->get(pixels->low())];
    const auto& rc2 = gfx::ColorTable[screenPalette->get(pixels->high())];

    *dest = (rc1.r << 16) | (rc1.g << 8) | (rc1.b) | 0xff000000;
    *(dest + 1) = (rc2.r << 16) | (rc2.g << 8) | (rc2.b) | 0xff000000;

    dest += 2;
  }
}

void Machine::color(color_t color)
{
  gfx::color_byte_t* penColor = _memory.penColor();
  penColor->low(color);
}

void Machine::cls(color_t color)
{
  color = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX)->get(color);
  gfx::color_byte_t value = gfx::color_byte_t(color, color);

  auto* data = _memory.screenData();
  memset(data, value.value, gfx::BYTES_PER_SCREEN);
}

void Machine::pset(coord_t x, coord_t y, color_t color)
{
  if (x >= 0 && x < gfx::SCREEN_WIDTH && y >= 0 && y < gfx::SCREEN_HEIGHT)
  {
    color = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX)->get(color);
    const auto& c = gfx::ColorTable[color];
    _memory.screenData(x, y)->set(x, color);
  }
}

color_t Machine::pget(coord_t x, coord_t y)
{
  return _memory.screenData(x, y)->get(x);
}

void Machine::line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
  // vertical
  if (y0 == y1)
  {
    if (x0 > x1) std::swap(x0, x1);

    for (coord_t x = x0; x <= x1; ++x)
      pset(x, y0, color);
  }
  // horizontal
  else if (x0 == x1)
  {
    if (y0 > y1) std::swap(y0, y1);

    for (coord_t y = y0; y <= y1; ++y)
      pset(x0, y, color);
  }
  else
  {
    float dx = x1 - (float)x0;
    float dy = y1 - y0;
    float derror = abs(dy / dx);
    float error = 0.0f;

    coord_t y = y0;
    for (coord_t x = x0; x <= x1; ++x)
    {
      pset(x, y, color);
      error += derror;
      if (error >= 0.5f)
      {
        y += copysignf(1, dy);
        error -= 1.0f;
      }
    }
  }

  // TODO: shouldn't be updated when invoked from other primitives, eg rect
  _state.lastLineEnd.x = x1;
  _state.lastLineEnd.x = y1;
}

void Machine::rect(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
  line(x0, y0, x1, y0, color);
  line(x1, y0, x1, y1, color);
  line(x0, y1, x1, y1, color);
  line(x0, y1, x0, y0, color);
}

void Machine::rectfill(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
  for (coord_t y = y0; y <= y1; ++y)
    for (coord_t x = x0; x < x1; ++x)
      pset(x, y, color);
}

void Machine::circHelper(coord_t xc, coord_t yc, coord_t x, coord_t y, color_t color)
{
  pset(xc + x, yc + y, color);
  pset(xc - x, yc + y, color);
  pset(xc + x, yc - y, color);
  pset(xc - x, yc - y, color);
  pset(xc + y, yc + x, color);
  pset(xc - y, yc + x, color);
  pset(xc + y, yc - x, color);
  pset(xc - y, yc - x, color);
}

void Machine::circ(coord_t xc, coord_t yc, amount_t r, color_t color)
{
  //TODO: not identical to pico-8 but acceptable for now
  coord_t x = 0, y = r;
  float d = 3 - 2 * r;
  circHelper(xc, yc, x, y, color);
  
  while (y >= x)
  {
    x++;

    if (d > 0)
    {
      y--;
      d = d + 4 * (x - y) + 10;
    }
    else
      d = d + 4 * x + 6;

    circHelper(xc, yc, x, y, color);
  }
}

void Machine::circfill(coord_t xc, coord_t yc, amount_t r, color_t color)
{
  //TODO: not identical to pico-8 but acceptable for now
  const amount_t sr = r * r;

  for (int x = -r; x < r; x++)
  {
    int hh = (int)std::sqrt(sr - x * x);
    int rx = xc + x;
    int ph = yc + hh;

    for (int y = yc - hh; y < ph; y++)
      pset(rx, y, color);
  }
}

void Machine::spr(index_t idx, coord_t x, coord_t y)
{
  const gfx::sprite_t* sprite = _memory.spriteAt(idx);

  for (coord_t ty = 0; ty < gfx::SPRITE_HEIGHT; ++ty)
    for (coord_t tx = 0; tx < gfx::SPRITE_WIDTH; ++tx)
    {
      color_t color = sprite->get(tx, ty);
      if (color != 0) //TODO: manage real transparency through flags
        pset(x + tx, y + ty, sprite->get(tx, ty));
    }

}

// TODO: fix strange characters like symbols
void Machine::print(const std::string& string, coord_t x, coord_t y, color_t color)
{
  for (const auto c : string)
  {
    const auto sprite = _font.glyph(c);

    for (coord_t ty = 0; ty < gfx::GLYPH_HEIGHT; ++ty)
      for (coord_t tx = 0; tx < gfx::GLYPH_WIDTH; ++tx)
      {
        //TODO: check if print is using pal override or not
        color_t fcolor = sprite->get(tx, ty);
        if (fcolor != 0)
          pset(x + tx, y + ty, color);
      }

    x += 4;
  }
}

void Machine::pal(color_t c0, color_t c1)
{
  gfx::palette_t* palette = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX);
  palette->set(c0, c1);
}
