#include "libretro.h"

#include "common.h"
#include "vm/gfx.h"

#include "io/loader.h"
#include "vm/machine.h"

#include <cstring>


retro8::Machine machine;
retro8::io::Loader loader;


extern "C"
{
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

  }
}

