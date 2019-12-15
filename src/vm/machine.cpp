#include "machine.h"

using namespace retro8;

void Machine::flip()
{
  auto* data = _memory.screenData();
  auto* screenPalette = _memory.paletteAt(gfx::SCREEN_PALETTE_INDEX);
  auto* dest = static_cast<uint32_t*>(_output->pixels);

  for (size_t i = 0; i < gfx::BYTES_PER_SCREEN; ++i)
  {
    const gfx::color_byte_t* pixels = data + i;    
    const auto rc1 = retro8::gfx::ColorTable::get(screenPalette->get((pixels)->low()));
    const auto rc2 = retro8::gfx::ColorTable::get(screenPalette->get((pixels)->high()));
      
    *(dest) = rc1;
    *((dest)+1) = rc2;
    (dest) += 2;
    
    //RASTERIZE_PIXEL_PAIR((*this), dest, pixels);
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

  _memory.clipRect()->reset();
  *_memory.cursor() = { 0, 0 };
}

void Machine::pset(coord_t x, coord_t y, color_t color)
{
  auto* clip = _memory.clipRect();
  x -= memory().camera()->x();
  y -= memory().camera()->y();

  if (x >= clip->x0 && x < clip->x1 && y >= clip->y0 && y < clip->y1)
  {
    color = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX)->get(color);
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
    coord_t dx = abs(x1 - x0);
    coord_t sx = x0 < x1 ? 1 : -1;
    coord_t dy = -abs(y1 - y0);
    coord_t sy = y0 < y1 ? 1 : -1;
    coord_t err = dx + dy;

    while (true)
    {
      pset(x0, y0, color);

      if (x0 == x1 && y0 == y1)
        break;

      coord_t err2 = 2 * err;

      if (err2 >= dy)
      {
        err += dy;
        x0 += sx;
      }

      if (err2 <= dx)
      {
        err += dx;
        y0 += sy;
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
    for (coord_t x = x0; x <= x1; ++x)
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
  const gfx::palette_t* palette = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX);

  for (coord_t ty = 0; ty < gfx::SPRITE_HEIGHT; ++ty)
    for (coord_t tx = 0; tx < gfx::SPRITE_WIDTH; ++tx)
    {
      color_t color = sprite->get(tx, ty);
      if (!palette->transparent(color))
        pset(x + tx, y + ty, color);
    }
}

void Machine::spr(index_t idx, coord_t bx, coord_t by, float sw, float sh, bool flipX, bool flipY)
{
  const gfx::palette_t* palette = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX);
  
  coord_t w = sw * gfx::SPRITE_WIDTH;
  coord_t h = sh * gfx::SPRITE_HEIGHT;

  /* we bypass spriteAt since we can use directly the address */
  const gfx::color_byte_t* base = reinterpret_cast<const gfx::color_byte_t*>(_memory.spriteAt(idx));
 
  for (coord_t y = 0; y < h; ++y)
  {
    for (coord_t x = 0; x < w; ++x)
    {
      coord_t fx = flipX ? (w - x - 1) : x;
      coord_t fy = flipY ? (h - y - 1) : y;
      
      //TODO: optimize by fetching only once if we need to read next pixel?
      const gfx::color_byte_t pair = *(base + y * gfx::SPRITE_SHEET_WIDTH_IN_BYTES + x / gfx::PIXEL_TO_BYTE_RATIO);
      const color_t color = pair.get(x);

      if (!palette->transparent(color))
        pset(bx + fx, by + fy, color);
    }
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

void Machine::pal(color_t c0, color_t c1, palette_index_t index)
{
  gfx::palette_t* palette = _memory.paletteAt(index);
  palette->set(c0, c1);
}


void Machine::map(coord_t cx, coord_t cy, coord_t x, coord_t y, amount_t cw, amount_t ch, sprite_flags_t layer)
{
  for (amount_t ty = 0; ty < ch; ++ty)
  {
    for (amount_t tx = 0; tx < cw; ++tx)
    {
      const sprite_index_t index = *_memory.spriteInTileMap(cx + tx, cy + ty);

      /* don't draw if index is 0 or layer is not zero and sprite flags are not correcly masked to it */
      if (index != 0 && (!layer || (layer & *_memory.spriteFlagsFor(index)) == layer))
        spr(index, x + tx * gfx::SPRITE_WIDTH, y + ty * gfx::SPRITE_HEIGHT);
    }
  }
}



