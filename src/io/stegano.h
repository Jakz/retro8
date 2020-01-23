#pragma once

#include "common.h"

#include "vm/machine.h"

namespace retro8
{
  namespace io
  {
    struct PngData
    {
      const uint32_t* data;
      void* userData;
      size_t length;
    };

#if R8_USE_LODE_PNG
    int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
#endif
    
    class Stegano
    {
    public:
      static constexpr size_t IMAGE_WIDTH = 160;
      static constexpr size_t IMAGE_HEIGHT = 205;

    private:
      uint8_t assembleByte(const uint32_t v);
    public:
      void load(const PngData& data, Machine& dest);
    };
  }
}