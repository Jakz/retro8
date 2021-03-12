#pragma once

#define PLATFORM_WIN32 0
#define PLATFORM_LIBRETRO 1
#define PLATFORM_OPENDINGUX 2
#define PLATFORM_FUNKEY 3

#define SOUND_ENABLED true

#if defined(FUNKEY_S)
#define PLATFORM PLATFORM_FUNKEY
#elif defined(__LIBRETRO__)
#define PLATFORM PLATFORM_LIBRETRO
#elif defined(_WIN32)
#define PLATFORM PLATFORM_WIN32
#endif

#define MOUSE_ENABLED false
#define TEST_MODE false

#define R8_OPTS_ENABLED true
#define R8_USE_LODE_PNG true

#if PLATFORM != PLATFORM_LIBRETRO

  #include "SDL.h"
  #define LOGD(x , ...) printf(x"\n", ## __VA_ARGS__)

  #if PLATFORM == PLATFORM_WIN32
    static constexpr int SCREEN_WIDTH = 240;
    static constexpr int SCREEN_HEIGHT = 240;

    #undef MOUSE_ENABLED
    #define MOUSE_ENABLED true

    static constexpr auto KEY_UP = SDLK_UP;
    static constexpr auto KEY_DOWN = SDLK_DOWN;
    static constexpr auto KEY_LEFT = SDLK_LEFT;
    static constexpr auto KEY_RIGHT = SDLK_RIGHT;

    static constexpr auto KEY_ACTION1_1 = SDLK_z;
    static constexpr auto KEY_ACTION1_2 = SDLK_x;
    static constexpr auto KEY_ACTION2_1 = SDLK_a;
    static constexpr auto KEY_ACTION2_2 = SDLK_s;

    static constexpr auto KEY_MUTE = SDLK_m;
    static constexpr auto KEY_PAUSE = SDLK_p;

    static constexpr auto KEY_NEXT_SCALER = SDLK_v;

    static constexpr auto KEY_MENU = SDLK_RETURN;
    static constexpr auto KEY_EXIT = SDLK_ESCAPE;

  #elif PLATFORM == PLATFORM_OPENDINGUX
    static constexpr int SCREEN_WIDTH = 320;
    static constexpr int SCREEN_HEIGHT = 240;

    static constexpr auto KEY_UP = SDLK_UP;
    static constexpr auto KEY_DOWN = SDLK_DOWN;
    static constexpr auto KEY_LEFT = SDLK_LEFT;
    static constexpr auto KEY_RIGHT = SDLK_RIGHT;

    static constexpr auto KEY_ACTION1_1 = SDLK_LCTRL; // A
    static constexpr auto KEY_ACTION1_2 = SDLK_LALT; // B
    static constexpr auto KEY_ACTION2_1 = SDLK_SPACE; // Y
    static constexpr auto KEY_ACTION2_2 = SDLK_LSHIFT; // X

    static constexpr auto KEY_MUTE = 0xffff;
    static constexpr auto KEY_PAUSE = 0xffff + 1;

    static constexpr auto KEY_NEXT_SCALER = SDLK_TAB; // L

    static constexpr auto KEY_MENU = SDLK_RETURN;
    static constexpr auto KEY_EXIT = SDLK_ESCAPE;

  #elif PLATFORM == PLATFORM_FUNKEY
    static constexpr int SCREEN_WIDTH = 240;
    static constexpr int SCREEN_HEIGHT = 240;

    static constexpr auto KEY_UP = SDLK_u;
    static constexpr auto KEY_DOWN = SDLK_d;
    static constexpr auto KEY_LEFT = SDLK_l;
    static constexpr auto KEY_RIGHT = SDLK_r;

    static constexpr auto KEY_ACTION1_1 = SDLK_b; // A
    static constexpr auto KEY_ACTION1_2 = SDLK_a; // B
    static constexpr auto KEY_ACTION2_1 = SDLK_y; // Y
    static constexpr auto KEY_ACTION2_2 = SDLK_x; // X

    static constexpr auto KEY_MUTE = 0xffff;
    static constexpr auto KEY_PAUSE = 0xffff + 1;

    static constexpr auto KEY_NEXT_SCALER = SDLK_h; // L

    static constexpr auto KEY_MENU = SDLK_s;
    static constexpr auto KEY_EXIT = 0xffff + 2;

  #endif

#else

#define LOGD(x, ...)

#endif
