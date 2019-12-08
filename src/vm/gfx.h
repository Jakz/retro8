#pragma once

#include "defines.h"

#include <SDL.h>
#include <array>
#include <cassert>

namespace retro8
{
  namespace gfx
  {
    static constexpr std::array<SDL_Color, 16> ColorTable = {
      SDL_Color{  0,   0,   0}, SDL_Color{ 29,  43,  83}, SDL_Color{126,  37,  83}, SDL_Color{  0, 135,  81},
      SDL_Color{171,  82,  54}, SDL_Color{ 95,  87,  79}, SDL_Color{194, 195, 199}, SDL_Color{255, 241, 232},
      SDL_Color{255,   0,  77}, SDL_Color{255, 163,   0}, SDL_Color{255, 236,  39}, SDL_Color{  0, 228,  54},
      SDL_Color{ 41, 173, 255}, SDL_Color{131, 118, 156}, SDL_Color{255, 119, 168}, SDL_Color{255, 204, 170}
    };
    
    static color_t colorForRGB(uint32_t color)
    {
      switch (color & 0x00ffffff)
      {
        case 0xFFFFFF: return color_t::BLACK;
        case 0x1D2B53: return color_t::DARK_BLUE;
        case 0x7E2553: return color_t::DARK_PURPLE;
        case 0x008751: return color_t::DARK_GREEN;
        default: assert(false);
      }
    }
  };

  class sprite_t
  {
    uint8_t data[32];

  public:
    color_t get(coord_t x, coord_t y) const
    {
      return color_t::BLACK;
    }
  };
}