#pragma once

#include <SDL.h>

#include <array>

namespace retro8
{
  enum color_t : uint8_t
  {
    BLACK, DARK_BLUE, DARK_PURPLE, DARK_GREEN,
    BROWN, DARK_GREY, LIGHT_GREY, WHITE,
    RED, ORANGE, YELLOW, GREEN,
    BLUE, INDIGO, PINK, PEACH
  };

  enum class button_t
  {
    LEFT    = 0x0001,
    RIGHT   = 0x0002,
    UP      = 0x0004,
    DOWN    = 0x0008,
    ACTION1 = 0x0010,
    ACTION2 = 0x0020,
  };

  using coord_t = int32_t;
  using amount_t = int32_t;
  using index_t = uint32_t;
  using sprite_index_t = uint8_t;
  using sprite_flags_t = uint8_t;
  using color_index_t = uint8_t;
  using palette_index_t = size_t;
  using address_t = int32_t;
  struct point_t { coord_t x, y; };
}

#define RASTERIZE_PIXEL_PAIR(machine, dest, pixels) do { \
  auto* screenPalette = (machine).memory().paletteAt(retro8::gfx::SCREEN_PALETTE_INDEX); \
  const auto rc1 = retro8::gfx::ColorTable::get(screenPalette->get((pixels)->low())); \
  const auto rc2 = retro8::gfx::ColorTable::get(screenPalette->get((pixels)->high())); \
\
  *(dest) = rc1; \
  *((dest)+1) = rc2; \
  (dest) += 2; \
  } \
  while (false)