#include "loader.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <sstream>

using namespace retro8;
using namespace retro8::io;



int LoaderP8::valueForHexDigit(char c)
{
  int v = (c >= 'A') ? (c >= 'a') ? (c - 'a' + 10) : (c - 'A' + 10) : (c - '0');
  assert(v >= 0 && v <= 0xf);
  return v;
}

color_t LoaderP8::colorFromDigit(char d)
{
  return static_cast<color_t>(valueForHexDigit(d));
}

sprite_index_t LoaderP8::spriteIndexFromString(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

uint8_t LoaderP8::valueForUint8(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

retro8::sprite_flags_t LoaderP8::spriteFlagsFromString(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

void LoaderP8::load(const std::string& path, Machine& m)
{
  std::vector<std::string> lines;

  std::ifstream input(path);
  assert(input.good());

  for (std::string line; std::getline(input, line); /**/)
    lines.push_back(line);

  for (auto& line : lines)
    if (!line.empty() && line.back() == '\r')
      line.resize(line.length() - 1);

  enum class State { HEADER, CODE, GFX, GFF, LABEL, MAP, SFX, MUSIC };

  State state = State::HEADER;

  //TODO: not efficient but for now it's fine
  std::stringstream code;

  coord_t sy = 0, my = 0, fy = 0, snd = 0;

  static constexpr size_t DIGITS_PER_PIXEL_ROW = 128;
  static constexpr size_t BYTES_PER_GFX_ROW = DIGITS_PER_PIXEL_ROW / 2;

  static constexpr size_t DIGITS_PER_MAP_ROW = 256;
  static constexpr size_t DIGITS_PER_SPRITE_FLAGS_ROW = 128*2;

  static constexpr size_t DIGITS_PER_SOUND = 168;


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
        //fixOperators(line);
        code << line << std::endl;
        break;

      case State::GFX:
        assert(line.length() == DIGITS_PER_PIXEL_ROW);
        for (coord_t x = 0; x < BYTES_PER_GFX_ROW; ++x)
        {
          const char* pair = line.c_str() + x * 2;
          auto* dest = m.memory().as<gfx::color_byte_t>(address::SPRITE_SHEET + sy * BYTES_PER_GFX_ROW + x);

          dest->setBoth(colorFromDigit(pair[0]), colorFromDigit(pair[1]));
        }

        ++sy;
        break;

      case State::MAP:
      {
        assert(line.length() == DIGITS_PER_MAP_ROW);
        for (coord_t x = 0; x < gfx::TILE_MAP_WIDTH; ++x)
        {
          const char* index = line.c_str() + x * 2;
          sprite_index_t sindex = spriteIndexFromString(index);
          *m.memory().spriteInTileMap(x, my) = sindex;
        }
        ++my;
        break;
      }
      case State::GFF:
      {
        assert(line.length() == DIGITS_PER_SPRITE_FLAGS_ROW);
        for (coord_t x = 0; x < DIGITS_PER_SPRITE_FLAGS_ROW/2; ++x)
        {
          const char* sflags = line.c_str() + x * 2;
          sprite_flags_t flags = spriteFlagsFromString(sflags);
          *m.memory().spriteFlagsFor(128*fy + x) = flags;
        }
        ++fy;
        break;
      }
      case State::SFX:
      {
        assert(line.length() == DIGITS_PER_SOUND);
        const char* p = line.c_str();

        sfx::Sound* sound = m.memory().sound(snd);
        sound->speed = valueForUint8(p+2); break;
        sound->loopStart = valueForUint8(p+4); break;
        sound->loopEnd = valueForUint8(p+6); break;

        p = p + 8;

        for (size_t i = 0; i < sound->samples.size(); ++i)
        {
          const char* s = p + (i * 5);
          
          auto& sample = sound->samples[i];
          
          sample.setPitch(valueForUint8(s));
          sample.setWaveform(sfx::Waveform(valueForHexDigit(s[2])));
          sample.setVolume(valueForHexDigit(s[3]));
          sample.setEffect(sfx::Effect(valueForHexDigit(s[4])));
        }

        ++snd;
        break;

      }
      }
    }
  }

  m.code().initFromSource(code.str());
}
