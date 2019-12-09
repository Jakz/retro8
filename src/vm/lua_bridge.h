#pragma once

#include <string>

struct lua_State;

namespace lua
{
  void registerFunctions(lua_State* state);

  class Code
  {
  private:
    lua_State* L;

  public:
    Code() : L(nullptr) { }
    ~Code();

    void initFromSource(const std::string& code);
    void callVoidFunction(const char* name);
  };
}