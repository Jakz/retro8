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
      retro8::color_t colorFromDigit(char d);

    public:
      void load(const std::string& path, Machine& dest);
    };
  }
}