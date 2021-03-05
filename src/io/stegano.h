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

    class Stegano
    {
    public:
      static constexpr size_t IMAGE_WIDTH = 160;
      static constexpr size_t IMAGE_HEIGHT = 205;

    private:
      uint8_t assembleByte(const uint32_t v);

      void load10(const PngData& data, Machine& dest);
      void load20(const PngData& data, Machine& dest);

    public:
      void load(const PngData& data, Machine& dest);
    };
  }
}