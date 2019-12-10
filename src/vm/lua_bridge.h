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

    bool _hasUpdate;
    bool _hasDraw;
    bool _require60fps;

  public:
    Code() : L(nullptr) { }
    ~Code();

    void initFromSource(const std::string& code);
    void callVoidFunction(const char* name);

    bool hasUpdate() const { return _hasUpdate; }
    bool hasDraw() const { return _hasDraw; }
    bool require60fps() const { return _require60fps; }

    void update();
    void draw();
     
  };
}