#include "loader.h"

#include "stegano.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <sstream>
//#include <cctype>

using namespace retro8;
using namespace retro8::io;

void Loader::fixLine(std::string& line)
{
  /* inline ? print operator */
  if (!line.empty() && line[0] == '?')
    line = "print(" + line.substr(1) + ")";
}

int Loader::valueForHexDigit(char c)
{
  int v = (c >= 'A') ? (c >= 'a') ? (c - 'a' + 10) : (c - 'A' + 10) : (c - '0');
  assert(v >= 0 && v <= 0xf);
  return v;
}

color_t Loader::colorFromDigit(char d)
{
  return static_cast<color_t>(valueForHexDigit(d));
}

sprite_index_t Loader::spriteIndexFromString(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

uint8_t Loader::valueForUint8(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

retro8::sprite_flags_t Loader::spriteFlagsFromString(const char* c)
{
  int h = valueForHexDigit(c[0]);
  int l = valueForHexDigit(c[1]);
  return (h << 4) | l;
}

bool Loader::isPngCartridge(const std::string& path)
{
  return path.length() >= 4 && path.substr(path.length() - 4) == ".png";
}

template<typename T>
std::vector<std::string> Loader::loadLines(T& input)
{
  std::vector<std::string> lines;

  for (std::string line; std::getline(input, line); /**/)
    lines.push_back(line);

  for (auto& line : lines)
    if (!line.empty() && line.back() == '\r')
      line.resize(line.length() - 1);

  return lines;
}

void Loader::loadFile(const std::string& path, Machine& dest)
{
  auto stream = std::ifstream(path);
  assert(stream.good());
  load(loadLines(stream), dest);
}

void Loader::loadRaw(const std::string& data, Machine& dest)
{
  auto stream = std::stringstream(data);
  load(loadLines(stream), dest);
}

void Loader::load(const std::vector<std::string>& lines, Machine& m)
{ 
  enum class State { HEADER, CODE, GFX, GFF, LABEL, MAP, SFX, MUSIC };

  State state = State::HEADER;

  //TODO: not efficient but for now it's fine
  std::stringstream code;

  coord_t sy = 0, my = 0, fy = 0, snd = 0, msc = 0;

  static constexpr size_t DIGITS_PER_PIXEL_ROW = 128;
  static constexpr size_t BYTES_PER_GFX_ROW = DIGITS_PER_PIXEL_ROW / 2;

  static constexpr size_t DIGITS_PER_MAP_ROW = 256;
  static constexpr size_t DIGITS_PER_SPRITE_FLAGS_ROW = 128*2;

  static constexpr size_t DIGITS_PER_SOUND = 168;
  static constexpr size_t DIGITS_PER_MUSIC_PATTERN = 2 + 1 + 8;


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
    else if (line.empty()) /* skip empty lines */
      continue;
    /*else if (std::all_of(line.begin(), line.end(), [](char c) { return std::isspace(c); }))
      continue;*/
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
        sound->speed = valueForUint8(p+2);
        sound->loopStart = valueForUint8(p+4);
        sound->loopEnd = valueForUint8(p+6);

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
      case State::MUSIC:
      {
        const char* p = line.c_str();

        if (!line.empty())
        {
          sfx::Music* music = m.memory().music(msc);

          /* XX AABBCCDD*/
          constexpr sfx::sound_index_t UNUSED_CHANNEL = 0x40;

          uint8_t flags = valueForUint8(p);

          if (flags & 0b1) music->markLoopBegin();
          else if (flags & 0b10) music->markLoopEnd();
          else if (flags & 0b100) music->markStop();

          for (sfx::channel_index_t i = 0; i < sfx::APU::CHANNEL_COUNT; ++i)
          {
            sfx::sound_index_t index = valueForUint8(p + 3 + 2 * i);

            if (index < UNUSED_CHANNEL)
              music->setSound(i, index);
            else
              assert(true || (index == UNUSED_CHANNEL + i + 1));
          }

          ++msc;

        }
        
        break;
      }
      }

    }
  }

  m.code().initFromSource(code.str());
}
