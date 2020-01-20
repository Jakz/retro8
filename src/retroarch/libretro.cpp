#include "libretro.h"

#include "common.h"
#include "vm/gfx.h"

#include "io/loader.h"
#include "io/stegano.h"
#include "vm/machine.h"

#include <cstring>

using pixel_t = uint32_t;


retro8::Machine machine;
retro8::io::Loader loader;
pixel_t* screen;

struct RetroArchEnv
{
  retro_video_refresh_t video;
};

RetroArchEnv env;


extern "C"
{
  unsigned retro_api_version()
  {
    return RETRO_API_VERSION;
  }

  void retro_init()
  {
    screen = new pixel_t[retro8::gfx::SCREEN_WIDTH * retro8::gfx::SCREEN_HEIGHT];
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
    info->geometry.aspect_ratio = retro8::gfx::TILE_MAP_WIDTH / float(retro8::gfx::TILE_MAP_HEIGHT);
  }

  void retro_set_environment(retro_environment_t env)
  {
    retro_pixel_format pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
    env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixelFormat);
  }

  void retro_set_video_refresh(retro_video_refresh_t callback)
  {
    env.video = callback;
  }

  bool retro_load_game(const retro_game_info* info)
  {
    if (info && info->data)
    {
      const uint8_t* bdata = static_cast<const uint8_t*>(info->data);

      if (std::memcmp(bdata, ".PNG", 4) == 0)
      {
        //TODO
      }
      else
      {
        //TODO
      }
      
      return true;
    }
  }

  void retro_run()
  {
    machine.code().update();
    machine.code().draw();
    machine.flip();
  }

  void retro_reset()
  {

  }
}

