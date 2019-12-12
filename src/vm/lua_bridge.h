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

    const void* _init;
    const void* _update;
    const void* _update60;
    const void* _draw;

  public:
    Code() : L(nullptr) { }
    ~Code();

    void loadAPI();
    void initFromSource(const std::string& code);
    void callVoidFunction(const char* name);

    bool hasUpdate() const { return _update != nullptr || _update60 != nullptr; }
    bool hasDraw() const { return _draw != nullptr; }
    bool require60fps() const { return _update60 != nullptr; }
    bool hasInit() const { return _init != nullptr; }

    void init();
    void update();
    void draw();
  };
}