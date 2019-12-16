#include "stegano.h"

#include <cassert>


using namespace retro8;
using namespace io;

constexpr size_t IMAGE_WIDTH = 160;
constexpr size_t IMAGE_HEIGHT = 205;

constexpr size_t RAW_DATA_LENGTH = 0x4300;

uint8_t Stegano::assembleByte(const uint32_t v)
{
  constexpr uint32_t MASK_ALPHA = 0xff00000;
  constexpr uint32_t MASK_RED = 0x000000ff;
  constexpr uint32_t MASK_GREEN = 0x0000ff00;
  constexpr uint32_t MASK_BLUE = 0x00ff0000;

  constexpr uint32_t SHIFT_ALPHA = 24;
  constexpr uint32_t SHIFT_RED = 0;
  constexpr uint32_t SHIFT_GREEN = 8;
  constexpr uint32_t SHIFT_BLUE = 16;

  return
    ((((v & MASK_ALPHA) >> SHIFT_ALPHA) & 0b11) ) |
    ((((v & MASK_RED) >> SHIFT_RED) & 0b11) << 2) |
    ((((v & MASK_GREEN) >> SHIFT_GREEN) & 0b11) << 4) |
    ((((v & MASK_BLUE) >> SHIFT_BLUE) & 0b11) << 6);
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

  /* first 0x4300 are read directly into the cart */
  for (size_t i = 0; i < RAW_DATA_LENGTH; ++i)
    m.memory().base()[i] = assembleByte(data.data[i]);

  size_t o = RAW_DATA_LENGTH;
  std::array<uint8_t, 4> magic;
  std::array<uint8_t, 4> expected = { { ':', 'c', ':', '\0' } };

  for (size_t i = 0; i < magic.size(); ++i)
    magic[i] = assembleByte(data.data[o++]);

  assert(magic == expected);
}
