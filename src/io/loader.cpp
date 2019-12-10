#include "loader.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <sstream>

using namespace retro8;
using namespace retro8::io;

color_t LoaderP8::colorFromDigit(char d)
{
  if (isdigit(d)) return static_cast<color_t>(d - '0');
  else if (d >= 'A' && d <= 'F') return static_cast<color_t>(d - 'F' + 10);
  else if (d >= 'a' && d <= 'f') return static_cast<color_t>(d - 'f' + 10);
  else assert(false);
}

//TODO: worst hacky solution ever, it's just a quick fix
void LoaderP8::fixOperators(std::string& line)
{
  size_t t = 0;
  size_t i0 = 0, i1 = 0;
  enum class S { BEGIN, IDENT, INBETWEEN, OPERATOR } s = S::BEGIN;

  while (t < line.length())
  {
    if (line[t] < 0)
      break;
    else if (isspace(line[t]))
    {
      if (s == S::BEGIN)
        ;
      else if (s == S::INBETWEEN)
        ;
      else if (s == S::IDENT)
      {
        i1 = t;
        s = S::INBETWEEN;
      }
      else
        break;
    }
    else if (isalnum(line[t]))
    {
      if (s == S::BEGIN)
      {
        s = S::IDENT;
        i0 = t;
      }
      else if (s == S::IDENT)
        ;
      else
        break;
    }
    else if (line[t] == '*' || line[t] == '+' || line[t] == '-' || line[t] == '/')
    {
      if (t < line.length() - 1 && line[t+1] == '=' && (s == S::IDENT || s == S::INBETWEEN)) 
      {
        if (s == S::IDENT)
          i1 = t - 1;
          
        // replace [i, t+1]
        line = line.substr(0, i0) + line.substr(i0, i1 - i0) + " = " + line.substr(i0, i1 - i0) + ' ' + line[t] + ' ' + line.substr(t + 2);
        break;
      }
      else
        break;
    }

    ++t;
  }
}

void LoaderP8::load(const std::string& path, Machine& m)
{
  std::vector<std::string> lines;

  std::ifstream input(path);
  for (std::string line; std::getline(input, line); /**/)
    lines.push_back(line);

  enum class State { HEADER, CODE, GFX, GFF, LABEL, MAP, SFX, MUSIC };
  
  State state = State::HEADER;

  //TODO: not efficient but for now it's fine
  std::stringstream code;

  coord_t sy = 0;

  static constexpr size_t DIGITS_PER_PIXEL_ROW = 128;
  static constexpr size_t BYTES_PER_GFX_ROW = DIGITS_PER_PIXEL_ROW / 2;

  for (auto& line : lines)
  {
    /* change state according to */
    if (line == "__lua__")
      state = State::CODE;
    else if (line == "__gfx__")
      state = State::GFX;
    else if (line == "__gff__")
      state = State::GFF;
    else if (line == "__label__")
      state = State::LABEL;
    else if (line == "__map__")
      state = State::MAP;
    else if (line == "__sfx__")
      state = State::SFX;
    else if (line == "__music__")
      state = State::MUSIC;
    else
    {
      switch (state)
      {
      case State::CODE:
        fixOperators(line);
        code << line << std::endl;
        break;

      case State::GFX:
        assert(line.length() == DIGITS_PER_PIXEL_ROW);
        for (size_t x = 0; x < BYTES_PER_GFX_ROW; ++x)
        {
          const char* pair = line.c_str() + x * 2;
          auto* dest = m.memory().as<gfx::color_byte_t>(address::SPRITE_SHEET + sy * BYTES_PER_GFX_ROW + x);

          dest->setBoth(colorFromDigit(pair[0]), colorFromDigit(pair[1]));
        }

        ++sy;
        break;
      }
    }
  }

  m.code().initFromSource(code.str());

}
