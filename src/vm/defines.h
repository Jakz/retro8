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
  using address_t = int32_t;
  struct point_t { coord_t x, y; };
}