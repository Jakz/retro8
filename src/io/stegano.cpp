#include "stegano.h"

#include <cassert>


using namespace retro8;
using namespace io;

constexpr size_t RAW_DATA_LENGTH = 0x4300;

#if DEBUGGER
#include <fstream>
static std::string fileName;
#endif

uint8_t Stegano::assembleByte(const uint32_t v)
{
  constexpr uint32_t MASK_ALPHA = 0xff000000;
  constexpr uint32_t MASK_RED = 0x00ff0000;
  constexpr uint32_t MASK_GREEN = 0x0000ff00;
  constexpr uint32_t MASK_BLUE = 0x000000ff;

  constexpr uint32_t SHIFT_ALPHA = 24;
  constexpr uint32_t SHIFT_RED = 16;
  constexpr uint32_t SHIFT_GREEN = 8;
  constexpr uint32_t SHIFT_BLUE = 0;

  return
    ((((v & MASK_ALPHA) >> SHIFT_ALPHA) & 0b11) << 6) |
    ((((v & MASK_RED) >> SHIFT_RED) & 0b11) << 0) |
    ((((v & MASK_GREEN) >> SHIFT_GREEN) & 0b11) << 2) |
    ((((v & MASK_BLUE) >> SHIFT_BLUE) & 0b11) << 4);
}

void Stegano::load(const PngData& data, Machine& m)
{
  constexpr size_t SPRITE_SHEET_SIZE = gfx::SPRITE_SHEET_HEIGHT * gfx::SPRITE_SHEET_WIDTH / gfx::PIXEL_TO_BYTE_RATIO;
  constexpr size_t TILE_MAP_SIZE = gfx::TILE_MAP_WIDTH * gfx::TILE_MAP_HEIGHT * sizeof(sprite_index_t) / 2;
  constexpr size_t SPRITE_FLAGS_SIZE = gfx::SPRITE_COUNT * sizeof(sprite_flags_t);
  constexpr size_t MUSIC_SIZE = sfx::MUSIC_COUNT * sizeof(sfx::music_t);
  constexpr size_t SOUND_SIZE = sfx::SOUND_COUNT * sizeof(sfx::sound_t);

  static_assert(sizeof(sfx::music_t) == 4, "Must be 4 bytes");
  static_assert(sizeof(sfx::sound_t) == 68, "Must be 68 bytes");

  static_assert(RAW_DATA_LENGTH == SPRITE_SHEET_SIZE + TILE_MAP_SIZE + SPRITE_FLAGS_SIZE + MUSIC_SIZE + SOUND_SIZE, "Must be equal");
  assert(data.length == IMAGE_WIDTH * IMAGE_HEIGHT);

  auto* d = data.data;

  /* first 0x4300 are read directly into the cart */
  for (size_t i = 0; i < RAW_DATA_LENGTH; ++i)
    m.memory().base()[i] = assembleByte(d[i]);

  size_t o = RAW_DATA_LENGTH;
  std::array<uint8_t, 4> magic;
  std::array<uint8_t, 4> expected = { { ':', 'c', ':', '\0' } };

  /* read magic code heaader */
  for (size_t i = 0; i < magic.size(); ++i)
    magic[i] = assembleByte(d[o++]);

  assert(magic == expected);

  /* read uint6_t lsb compressed length*/
  size_t compressedLength = assembleByte(d[o]) << 8 | assembleByte(d[o + 1]);
  o += 2;
  /* skip 2 null*/
  o += 2;

  compressedLength = std::min(size_t(32769ULL - RAW_DATA_LENGTH), compressedLength);

  const std::string lookup = "\n 0123456789abcdefghijklmnopqrstuvwxyz!#%(){}[]<>+=/*:;.,~_";
  std::string code;

  assert(0x3b == lookup.length());
  //TODO: optimize concatenation on string by reserving space

  for (size_t i = 0; i < compressedLength; ++i)
  {
    uint8_t v = assembleByte(d[o + i]);

    /* copy next */
    if (v == 0x00)
    {
      uint8_t vn = assembleByte(d[o + i + 1]);
      code += vn;
      ++i;
    }
    /* lookup */
    else if (v <= 0x3b)
    {
      code += lookup[v - 1];
    }
    else
    {
      uint8_t vn = assembleByte(d[o + i + 1]);

      auto offset = ((v - 0x3c) << 4) + (vn & 0xf);
      auto length = (vn >> 4) + 2;

      const size_t start = code.length() - offset;
      for (size_t j = 0; j < length; ++j)
        code += code[start + j];

      ++i;
    }
  }

#if DEBUGGER
  std::ofstream output(fileName);
  output << code;
  output.close();
#endif

  m.code().initFromSource(code);
}
