#include "common.h"

#include "vm/machine.h"

#include <vector>
#include <string>

namespace retro8
{
  namespace io
  {
    class LoaderP8
    {
    private:
      void fixOperators(std::string& code);

      int valueForHexDigit(char c);
      retro8::color_t colorFromDigit(char d);
      retro8::sprite_index_t spriteIndexFromString(const char* c);
      retro8::sprite_flags_t spriteFlagsFromString(const char* c);

    public:
      void load(const std::string& path, Machine& dest);
    };
  }
}