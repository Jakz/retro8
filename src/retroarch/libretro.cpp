#include "libretro.h"

#include "common.h"
#include "vm/gfx.h"

#include "io/loader.h"
#include "io/stegano.h"
#include "vm/machine.h"

#include <cstring>

namespace r8 = retro8;
using pixel_t = uint32_t;


r8::Machine machine;
r8::io::Loader loader;

r8::gfx::ColorTable colorTable;
pixel_t* screen;

struct RetroArchEnv
{
  retro_video_refresh_t video;
  retro_audio_sample_t audio;
  retro_audio_sample_batch_t audioBatch;
  retro_input_poll_t inputPoll;
  retro_input_state_t inputState;
};

RetroArchEnv env;

struct ColorMapper
{
  r8::gfx::ColorTable::pixel_t operator()(uint8_t r, uint8_t g, uint8_t b) const { return 0xff000000 | (r << 16) | (g << 8) | b; }
};

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);

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
    LOGD("Initializing screen buffer of %zu bytes", sizeof(pixel_t)*r8::gfx::SCREEN_WIDTH*r8::gfx::SCREEN_HEIGHT);
    colorTable.init(ColorMapper());
  }

  void retro_deinit()
  {
    delete[] screen;
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
    info->timing.sample_rate = 44100;
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
      machine.code().loadAPI();

      
      const char* bdata = static_cast<const char*>(info->data);

      if (std::memcmp(bdata, ".PNG", 4) == 0)
      {
        std::vector<uint8_t> out;
        unsigned long width, height;
        auto result = decodePNG(out, width, height, (uint8_t*)bdata, info->size, true);
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
          LOGD("Cartridge has _init() function, calling it.");
          machine.code().init();
          LOGD("_init() function completed execution.");
        //});
      }

      machine.sound().init();
 
      return true;
    }
  }

  void retro_unload_game(void)
  {
    /* TODO */
  }

  void retro_run()
  {
    machine.code().update();
    machine.code().draw();
    
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

    env.video(screen, r8::gfx::SCREEN_WIDTH, r8::gfx::SCREEN_HEIGHT, r8::gfx::SCREEN_WIDTH * sizeof(pixel_t));

  }

  void retro_reset()
  {

  }
}

