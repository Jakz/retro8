#include "machine.h"

#include <algorithm>

using namespace retro8;

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
    color = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX)->get(color_t(color % gfx::COLOR_COUNT));
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
#if R8_OPTS_ENABLED

  /* compute directly actual bounding box and set the rect without invoking pset */

  auto* clip = _memory.clipRect();
  auto cx = memory().camera()->x(), cy = memory().camera()->y();

  x0 -= cx;
  y0 -= cy;
  x1 -= cx;
  y1 -= cy;

  x0 = std::max(x0, coord_t(clip->x0));
  x1 = std::min(x1, coord_t(clip->x1));
  y0 = std::max(y0, coord_t(clip->y0));
  y1 = std::min(y1, coord_t(clip->y1));

  color = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX)->get(color_t(color % gfx::COLOR_COUNT));

  for (coord_t y = y0; y <= y1; ++y)
    for (coord_t x = x0; x <= x1; ++x)
      _memory.screenData(x, y)->set(x, color);
#else
  for (coord_t y = y0; y <= y1; ++y)
    for (coord_t x = x0; x <= x1; ++x)
      pset(x, y, color);
#endif
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
      d = d + 4 * (x - y) + 5;
    }
    else
      d = d + 4 * x + 3;

    circHelper(xc, yc, x, y, color);
  }
}

void Machine::circFillHelper(coord_t xc, coord_t yc, coord_t x, coord_t y, color_t col)
{
  //TODO: totally inefficient
  rectfill(xc - x, yc - y, xc + x, yc + y, col);
  rectfill(xc - y, yc - x, xc + y, yc + x, col);
}


void Machine::circfill(coord_t xc, coord_t yc, amount_t r, color_t color)
{
  //TODO: not identical to pico-8 but acceptable for now
  coord_t x = 0, y = r;
  float d = 3 - 2 * r;
  circFillHelper(xc, yc, x, y, color);

  int ctr = 0;
  while (y >= x)
  {
    x++;

    if (d > 0)
    {
      y--;
      d = d + 3 * (x - y) + 5;
    }
    else
      d = d + 3 * x + 3;

    circFillHelper(xc, yc, x, y, color);
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
      const gfx::color_byte_t pair = *(base + y * gfx::SPRITE_SHEET_PITCH + x / gfx::PIXEL_TO_BYTE_RATIO);
      const color_t color = pair.get(x);

      if (!palette->transparent(color))
        pset(bx + fx, by + fy, color);
    }
  }
}

void Machine::sspr(coord_t sx, coord_t sy, coord_t sw, coord_t sh, coord_t dx, coord_t dy, coord_t dw, coord_t dh, bool flipX, bool flipY)
{
  const gfx::palette_t* palette = _memory.paletteAt(gfx::DRAW_PALETTE_INDEX);

  float fx = sx, fy = sy;
  float xr = sw / float(dw);
  float yr = sh / float(dh);

  //TODO: flipx flipy, test ratio calculation

  for (coord_t y = 0; y < dh; ++y)
  {
    for (coord_t x = 0; x < dw; ++x)
    {
      coord_t cx = fx, cy = fy;
      auto pair = _memory.spriteSheet(cx, cy);
      auto color = pair->get(cx);

      if (!palette->transparent(color))
        pset(dx + x, dy + y, color);

      fx += xr;
    }

    fx = sx;
    fy += yr;
  }
}

// TODO: add support for strange characters like symbols
void Machine::print(const std::string& string, coord_t x, coord_t y, color_t color)
{
  
  struct SpecialGlyph
  {
    std::vector<uint8_t> encoding;
    size_t index;
  };

  static const std::array<SpecialGlyph, 12> SpecialGlyphs = { {
    { { 0xe2, 0xac, 0x87, 0xef, 0xb8, 0x8f }, 3 }, // down arrow
    { { 0xe2, 0xac, 0x86, 0xef, 0xb8, 0x8f }, 20}, // up arrow
    { { 0xe2, 0xac, 0x85, 0xef, 0xb8, 0x8f }, 11}, // left arrow
    { { 0xe2, 0x9e, 0xa1, 0xef, 0xb8, 0x8f }, 17}, // right arrow
    { { 0xf0, 0x9f, 0x85, 0xbe, 0xef, 0xb8, 0x8f }, 14 }, // o button
    { { 0xe2, 0x9d, 0x8e }, 23 }, // x button

    // 0x8b left, 0x91 right, 0x94 up, 0x83 down, 0x83 o, 0x97 x
    { { 0x8b }, 11 }, { { 0x91, }, 17 }, { { 0x94 }, 20 }, { { 0x83 }, 3 }, { { 0x8e }, 14 }, { { 0x97 }, 23 } 
  } };

  static const std::array<uint8_t, 2> Prefixes = { 0xe2, 0xf0 };

  const coord_t sx = x;
  for (size_t i = 0; i < string.length(); ++i)
  {
    auto c = string[i];

    if (c == '\n')
    {
      y += TEXT_LINE_HEIGHT;
      x = sx;
      continue;
    }

    auto specialGlyph = std::find_if(SpecialGlyphs.begin(), SpecialGlyphs.end(), [&string, &i](const SpecialGlyph& glyph) {
      return string.size() >= i + glyph.encoding.size() && memcmp(&string[i], &glyph.encoding[0], glyph.encoding.size()) == 0; //TODO: memcmp is not best design ever
    });

    const gfx::sequential_sprite_t* sprite = nullptr;
    coord_t width = gfx::GLYPH_WIDTH;

    if (specialGlyph != SpecialGlyphs.end())
    {
      sprite = _font.specialGlyph(specialGlyph->index);
      width = 8;
      i += specialGlyph->encoding.size() - 1;
    }
    else
      sprite = _font.glyph(c);

    if (sprite)
    {
      for (coord_t ty = 0; ty < gfx::GLYPH_HEIGHT; ++ty)
        for (coord_t tx = 0; tx < width; ++tx)
        {
          color_t fcolor = sprite->get(tx, ty);
          if (fcolor != 0)
            pset(x + tx, y + ty, color);
        }

      x += width;
    }
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
      /* TODO: experimentally the behavior is layer & flags != 0 instead that layer & flags == layer */
      if (index != 0 && (!layer || (layer & *_memory.spriteFlagsFor(index)) != 0))
        spr(index, x + tx * gfx::SPRITE_WIDTH, y + ty * gfx::SPRITE_HEIGHT);
    }
  }
}
