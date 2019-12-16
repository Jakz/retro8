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
      size_t length;
    };
    
    class Stegano
    {
    private:
      uint8_t assembleByte(const uint32_t v);
    public:
      void load(const PngData& data, Machine& dest);
    };
  }
}