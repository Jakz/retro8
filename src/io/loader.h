#include "common.h"

#include "vm/machine.h"

#include <vector>
#include <string>

namespace retro8
{
  namespace io
  {
    class Loader
    {
    private:
      int valueForHexDigit(char c);
      retro8::color_t colorFromDigit(char d);
      retro8::sprite_index_t spriteIndexFromString(const char* c);
      retro8::sprite_flags_t spriteFlagsFromString(const char* c);
      uint8_t valueForUint8(const char* c);

    public:
      void load(const std::string& path, Machine& dest);

      std::string load(const std::string& path);

      static bool isPngCartridge(const std::string& path);

      static void fixLine(std::string& line);

      
    };
  }
}