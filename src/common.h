#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

using u32 = uint32_t;
using u16 = uint16_t;

using s32 = int32_t;
using s64 = int64_t;

using byte = uint8_t;

template<typename T>
struct bit_mask
{
  using utype = typename std::underlying_type<T>::type;
  utype value;

  inline bool isSet(T flag) const { return value & static_cast<utype>(flag); }
  inline void set(T flag) { value |= static_cast<utype>(flag); }
  inline void reset(T flag) { value &= ~static_cast<utype>(flag); }
  inline void set(T flag, bool value) { if (value) set(flag); else reset(flag); }

  inline bit_mask<T> operator~() const
  {
    return bit_mask<T>(~value);
  }

  inline bit_mask<T> operator&(T flag) const
  {
    return bit_mask<T>(value & static_cast<utype>(flag));

  }

  inline bit_mask<T> operator|(T flag) const
  {
    return bit_mask<T>(value | static_cast<utype>(flag));
  }

  inline bit_mask<T> operator&(const bit_mask<T>& other) const
  {
    return bit_mask<T>(value & other.value);
  }

  bit_mask<T>() : value(0) { }

private:
  bit_mask<T>(utype value) : value(value) { }
};

struct Platform
{
  static uint32_t getTicks();
  static int loadPNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
};

#define SOUND_ENABLED true

#ifdef _WIN32
#define LOGD(x , ...) printf(x"\n", __VA_ARGS__)
#else
#define LOGD(...)
#endif

#define R8_OPTS_ENABLED true
#define R8_USE_LODE_PNG true

#ifdef _WIN32
#define MOUSE_ENABLED true
#define WINDOW_SCALE 1

#if SDL_COMPILEDVERSION > 2000
#define DEBUGGER true
#endif
#define DESKTOP_MODE true
#define TEST_MODE false
#else
#define MOUSE_ENABLED false
#define DESKTOP_MODE false
#endif
