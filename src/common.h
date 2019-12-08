#pragma once

#include <cstdint>

using u32 = uint32_t;
using u16 = uint16_t;

using s32 = int32_t;
using s64 = int64_t;

using byte = uint8_t;


#ifdef _WIN32
#define MOUSE_ENABLED true
#define WINDOW_SCALE 1
#else
#define MOUSE_ENABLED false
#endif