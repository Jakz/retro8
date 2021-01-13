#include "libretro.h"

#include "common.h"
#include "vm/gfx.h"

#include "io/loader.h"
#include "io/stegano.h"
#include "vm/machine.h"
#include "vm/input.h"

#include <cstring>

#define LIBRETRO_LOG LOGD

namespace r8 = retro8;
using pixel_t = uint32_t;

constexpr int SAMPLE_RATE = 44100;
constexpr int SAMPLES_PER_FRAME = SAMPLE_RATE / 15;

r8::Machine machine;
r8::io::Loader loader;

r8::input::InputManager input;
r8::gfx::ColorTable colorTable;
pixel_t* screen;
int16_t* audioBuffer;

struct RetroArchEnv
{
  retro_video_refresh_t video;
  retro_audio_sample_t audio;
  retro_audio_sample_batch_t audioBatch;
  retro_input_poll_t inputPoll;
  retro_input_state_t inputState;

  uint32_t frameCounter;
  uint16_t buttonState;
};

RetroArchEnv env;

struct ColorMapper
{
  r8::gfx::ColorTable::pixel_t operator()(uint8_t r, uint8_t g, uint8_t b) const { return 0xff000000 | (r << 16) | (g << 8) | b; }
};

//TODO
uint32_t Platform::getTicks() { return 0; }

extern "C"
{
  unsigned retro_api_version()
  {
    return RETRO_API_VERSION;
  }

  void retro_init()
  {
    screen = new pixel_t[r8::gfx::SCREEN_WIDTH * r8::gfx::SCREEN_HEIGHT];
    LIBRETRO_LOG("Initializing screen buffer of %zu bytes", sizeof(pixel_t)*r8::gfx::SCREEN_WIDTH*r8::gfx::SCREEN_HEIGHT);

    audioBuffer = new int16_t[SAMPLES_PER_FRAME * 2];
    LIBRETRO_LOG("Initializing audio buffer of %zu bytes", sizeof(int16_t) * SAMPLES_PER_FRAME * 2);

    colorTable.init(ColorMapper());
    machine.font().load();
    machine.code().loadAPI();
    input.setMachine(&machine);
  }

  void retro_deinit()
  {
    delete[] screen;
    delete[] audioBuffer;
    //TODO: release all structures bound to Lua etc
  }

  void retro_get_system_info(retro_system_info* info)
  {
    std::memset(info, 0, sizeof(info));

    info->library_name = "retro-8";
    info->library_version = "0.1b";
    info->need_fullpath = false;
    info->valid_extensions = "p8|png";
  }

  void retro_get_system_av_info(retro_system_av_info* info)
  {
    info->timing.fps = 60.0f;
    info->timing.sample_rate = SAMPLE_RATE;
    info->geometry.base_width = retro8::gfx::SCREEN_WIDTH;
    info->geometry.base_height = retro8::gfx::SCREEN_HEIGHT;
    info->geometry.max_width = retro8::gfx::SCREEN_WIDTH;
    info->geometry.max_height = retro8::gfx::SCREEN_HEIGHT;
    info->geometry.aspect_ratio = retro8::gfx::SCREEN_WIDTH / float(retro8::gfx::SCREEN_HEIGHT);
  }

  void retro_set_environment(retro_environment_t env)
  {
    retro_pixel_format pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
    env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixelFormat);
  }

  void retro_set_video_refresh(retro_video_refresh_t callback) { env.video = callback; }
  void retro_set_audio_sample(retro_audio_sample_t callback) { env.audio = callback; }
  void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback) { env.audioBatch = callback; }
  void retro_set_input_poll(retro_input_poll_t callback) { env.inputPoll = callback; }
  void retro_set_input_state(retro_input_state_t callback) { env.inputState = callback; }
  void retro_set_controller_port_device(unsigned port, unsigned device) { /* TODO */ }


  size_t retro_serialize_size(void) { return 0; }
  bool retro_serialize(void *data, size_t size) { return true; }
  bool retro_unserialize(const void *data, size_t size) { return true; }
  void retro_cheat_reset(void) { }
  void retro_cheat_set(unsigned index, bool enabled, const char *code) { }
  unsigned retro_get_region(void) { return 0; }
  void *retro_get_memory_data(unsigned id) { return nullptr; }
  size_t retro_get_memory_size(unsigned id) { return 0; }

  bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }
  bool retro_load_game(const retro_game_info* info)
  {
    if (info && info->data)
    {
      input.reset();

      const char* bdata = static_cast<const char*>(info->data);

      LIBRETRO_LOG("[Retro8] Loading %s", info->path);


      if (std::memcmp(bdata, "\x89PNG", 4) == 0)
      {
        LIBRETRO_LOG("[Retro8] Game is in PNG format, decoding it.");

        std::vector<uint8_t> out;
        unsigned long width, height;
        auto result = Platform::loadPNG(out, width, height, (uint8_t*)bdata, info->size, true);
        assert(result == 0);
        r8::io::Stegano stegano;
        stegano.load({ reinterpret_cast<const uint32_t*>(out.data()), nullptr, out.size() / 4 }, machine);
      }
      else
      {
        //TODO: not efficient since it's copied and it's not checking for '\0'
        std::string raw(bdata);
        loader.loadRaw(raw, machine);
      }

      machine.memory().backupCartridge();

      if (machine.code().hasInit())
      {
        //_initFuture = std::async(std::launch::async, []() {
        LIBRETRO_LOG("[Retro8] Cartridge has _init() function, calling it.");
          machine.code().init();
          LIBRETRO_LOG("[Retro8] _init() function completed execution.");
        //});
      }

      machine.sound().init();

      env.frameCounter = 0;

      return true;
    }

    return false;
  }

  void retro_unload_game(void)
  {
    /* TODO */
  }

  void retro_run()
  {
    /* if code is at 60fps or every 2 frames (30fps) */
    if (machine.code().require60fps() || env.frameCounter % 2 == 0)
    {
      /* call _update and _draw of PICO-8 code */
      machine.code().update();
      machine.code().draw();

      /* rasterize screen memory to ARGB framebuffer */
      auto* data = machine.memory().screenData();
      auto* screenPalette = machine.memory().paletteAt(retro8::gfx::SCREEN_PALETTE_INDEX);

      auto pointer = screen;

      for (size_t i = 0; i < r8::gfx::BYTES_PER_SCREEN; ++i)
      {
        const r8::gfx::color_byte_t* pixels = data + i;
        const auto rc1 = colorTable.get(screenPalette->get((pixels)->low()));
        const auto rc2 = colorTable.get(screenPalette->get((pixels)->high()));

        *(pointer) = rc1;
        *((pointer)+1) = rc2;
        (pointer) += 2;
      }

      input.manageKeyRepeat();
    }

    env.video(screen, r8::gfx::SCREEN_WIDTH, r8::gfx::SCREEN_HEIGHT, r8::gfx::SCREEN_WIDTH * sizeof(pixel_t));
    ++env.frameCounter;

    machine.sound().renderSounds(audioBuffer, SAMPLES_PER_FRAME);
    env.audioBatch(audioBuffer, SAMPLES_PER_FRAME);

    /* manage input */
    {
      const unsigned player = 0;

      struct BtPair {
        int16_t rabt;
        size_t r8bt;
        bool isSet;
      };

      static std::array<BtPair, retro8::BUTTON_COUNT> mapping = { {
        { RETRO_DEVICE_ID_JOYPAD_LEFT, 0, false },
        { RETRO_DEVICE_ID_JOYPAD_RIGHT, 1, false },
        { RETRO_DEVICE_ID_JOYPAD_UP, 2, false },
        { RETRO_DEVICE_ID_JOYPAD_DOWN, 3, false },
        { RETRO_DEVICE_ID_JOYPAD_A, 4, false },
        { RETRO_DEVICE_ID_JOYPAD_B, 5, false },
      } };


      //TODO: add mapping for action1/2 of player 2 because it's used by some games

      env.inputPoll();
      for (auto& entry : mapping)
      {
        const bool isSet = env.inputState(player, RETRO_DEVICE_JOYPAD, 0, entry.rabt);
        const bool wasSet = entry.isSet;

        if (wasSet != isSet)
          input.manageKey(player, entry.r8bt, isSet);

        entry.isSet = isSet;
      }

      input.tick();
    }

  }

  void retro_reset()
  {

  }
}

