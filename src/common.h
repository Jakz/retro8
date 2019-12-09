#pragma once

#include <cstdint>
#include <type_traits>

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

  bit_mask<T> operator&(T flag) const
  {
    bit_mask<T> mask;
    mask.value = this->value & static_cast<utype>(flag);
    return mask;
  }

  bit_mask<T> operator|(T flag) const
  {
    bit_mask<T> mask;
    mask.value = this->value | static_cast<utype>(flag);
    return mask;
  }
};


#ifdef _WIN32
#define MOUSE_ENABLED true
#define WINDOW_SCALE 1
#else
#define MOUSE_ENABLED false
#endif