#pragma once

#include "common.h"
#include "defines.h"
#include "gfx.h"
#include "sound.h"
#include "lua_bridge.h"

#include <array>
#include <random>
#include <cstring>

namespace retro8
{
  namespace address
  {
    static constexpr address_t SPRITE_SHEET = 0x0000;
    static constexpr address_t SPRITE_FLAGS = 0x3000;

    static constexpr address_t MUSIC = 0x3100;
    static constexpr address_t SOUNDS = 0x3200;

    static constexpr address_t CART_DATA = 0x5e00;
    static constexpr address_t PALETTES = 0x5f00;
    static constexpr address_t CLIP_RECT = 0x5f20;
    static constexpr address_t PEN_COLOR = 0x5f25;
    static constexpr address_t CURSOR = 0x5f26;
    static constexpr address_t CAMERA = 0x5f28;

    static constexpr address_t SCREEN_DATA = 0x6000;

    static constexpr address_t TILE_MAP_LOW = 0x1000;
    static constexpr address_t TILE_MAP_HIGH = 0x2000;

    static constexpr int32_t CART_DATA_LENGTH = 0x4300;
  };

  class Memory
  {
  private:
    uint8_t _backup[address::CART_DATA_LENGTH];
    uint8_t memory[1024 * 32];

    static constexpr size_t BYTES_PER_PALETTE = sizeof(retro8::gfx::palette_t);
    static constexpr size_t BYTES_PER_SPRITE = sizeof(retro8::gfx::sprite_t);

    static constexpr size_t ROWS_PER_TILE_MAP_HALF = 32;


  public:
    Memory()
    {
      memset(memory, 0, 1024 * 32);
      paletteAt(gfx::DRAW_PALETTE_INDEX)->reset();
      paletteAt(gfx::SCREEN_PALETTE_INDEX)->reset();
      clipRect()->reset();
      cursor()->reset();
    }

    void backupCartridge()
    {
      std::memcpy(_backup, memory, address::CART_DATA_LENGTH);
    }

    const uint8_t* backup() const { return _backup; }
    uint8_t* base() { return memory; }

    gfx::color_byte_t* penColor() { return as<gfx::color_byte_t>(address::PEN_COLOR); }
    gfx::cursor_t* cursor() { return as<gfx::cursor_t>(address::CURSOR); }
    gfx::camera_t* camera() { return as<gfx::camera_t>(address::CAMERA); }
    gfx::clip_rect_t* clipRect() { return as<gfx::clip_rect_t>(address::CLIP_RECT); }

    gfx::color_byte_t* spriteSheet(coord_t x, coord_t y) { return spriteSheet() + x / gfx::PIXEL_TO_BYTE_RATIO + y * gfx::SPRITE_SHEET_PITCH; }
    gfx::color_byte_t* spriteSheet() { return as<gfx::color_byte_t>(address::SPRITE_SHEET); }
    gfx::color_byte_t* screenData() { return as<gfx::color_byte_t>(address::SCREEN_DATA); }
    gfx::color_byte_t* screenData(coord_t x, coord_t y) { return screenData() + y * gfx::SCREEN_PITCH + x / gfx::PIXEL_TO_BYTE_RATIO; }
    integral_t* cartData(index_t idx) { return as<integral_t>(address::CART_DATA + idx * sizeof(integral_t)); } //TODO: ENDIANNESS!!

    sfx::Sound* sound(sfx::sound_index_t i) { return as<sfx::Sound>(address::SOUNDS + sizeof(sfx::Sound)*i); }
    sfx::Music* music(sfx::music_index_t i) { return as<sfx::Music>(address::MUSIC + sizeof(sfx::Music)*i); }

    sprite_flags_t* spriteFlagsFor(sprite_index_t index)
    {
      return as<sprite_flags_t>(address::SPRITE_FLAGS + index);
    }

    sprite_index_t* spriteInTileMap(coord_t x, coord_t y)
    {
      static_assert(sizeof(sprite_index_t) == 1, "sprite_index_t must be 1 byte");

      sprite_index_t *addr;

      if (y >= ROWS_PER_TILE_MAP_HALF)
        addr = as<sprite_index_t>(address::TILE_MAP_LOW) + x + (y - ROWS_PER_TILE_MAP_HALF) * gfx::TILE_MAP_WIDTH * sizeof(sprite_index_t);
      else
        addr = as<sprite_index_t>(address::TILE_MAP_HIGH) + x + y * gfx::TILE_MAP_WIDTH * sizeof(sprite_index_t);

      assert((addr >= memory + address::TILE_MAP_LOW && addr <= memory + address::TILE_MAP_LOW * gfx::TILE_MAP_WIDTH * gfx::TILE_MAP_HEIGHT * sizeof(sprite_index_t)));

      return addr;
    }

    gfx::sprite_t* spriteAt(sprite_index_t index) {
      return reinterpret_cast<gfx::sprite_t*>(&memory[address::SPRITE_SHEET
        + (index % gfx::SPRITES_PER_SPRITE_SHEET_ROW) * gfx::SPRITE_BYTES_PER_SPRITE_ROW]
        + (index / gfx::SPRITES_PER_SPRITE_SHEET_ROW) * gfx::SPRITE_SHEET_PITCH * gfx::SPRITE_HEIGHT
        ); }
    gfx::palette_t* paletteAt(palette_index_t index) { return reinterpret_cast<gfx::palette_t*>(&memory[address::PALETTES + index * BYTES_PER_PALETTE]); }

    template<typename T> T* as(address_t addr) { return reinterpret_cast<T*>(&memory[addr]); }
  };
}
