#pragma once

#include <SDL.h>

#include <array>

namespace retro8
{
  enum color_t
  {
    BLACK, DARK_BLUE, DARK_PURPLE, DARK_GREEN,
    BROWN, DARK_GREY, LIGHT_GREY, WHITE,
    RED, ORANGE, YELLOW, GREEN,
    BLUE, INDIGO, PINK, PEACH
  };

  static constexpr std::array<SDL_Color, 16> ColorTable = {
    SDL_Color{  0,   0,   0}, SDL_Color{ 29,  43,  83}, SDL_Color{126,  37,  83}, SDL_Color{  0, 135,  81},
    SDL_Color{171,  82,  54}, SDL_Color{ 95,  87,  79}, SDL_Color{194, 195, 199}, SDL_Color{255, 241, 232},
    SDL_Color{255,   0,  77}, SDL_Color{255, 163,   0}, SDL_Color{255, 236,  39}, SDL_Color{  0, 228,  54},
    SDL_Color{ 41, 173, 255}, SDL_Color{131, 118, 156}, SDL_Color{255, 119, 168}, SDL_Color{255, 204, 170}
  };

  using coord_t = uint32_t;
  struct point_t { coord_t x, y; };

  class State
  {
  public:
    color_t penColor;
    point_t lastLineEnd;
  };

  class Memory
  {
  private:
    uint8_t memory[1024 * 32];

    static constexpr size_t SCREEN_DATA = 0x6000;

    static constexpr size_t BYTES_PER_SCREEN_ROW = 128;
  public:


    void setScreenData(coord_t x, coord_t y, color_t c)
    {
      uint8_t* address = &memory[SCREEN_DATA + y * BYTES_PER_SCREEN_ROW + x / 2];

      if (x % 2 == 0)
        *address = (*address & 0xf0) | c;
      else
        *address = (*address & 0x0f) | (c << 4);
    }

    color_t screenData(coord_t x, coord_t y) const
    {
      const uint8_t* address = &memory[SCREEN_DATA + y * BYTES_PER_SCREEN_ROW + x / 2];

      if (x % 2 == 0)
        return static_cast<color_t>(*address & 0x0f);
      else
        return static_cast<color_t>((*address >> 4) & 0x0f);
    }

  };

  class Machine
  {
  private:
    State _state;
    Memory _memory;
    SDL_Surface* _surface;

  public:
    Machine()
    {
      _surface = SDL_CreateRGBSurface(0, 128, 128, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    }

    void color(color_t color);

    void pset(coord_t x, coord_t y, color_t color);
    color_t pget(coord_t x, coord_t y);

    void line(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
    void rect(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);

    const State& state() const { return _state; }
    SDL_Surface* screen() const { return _surface; }
  };
}