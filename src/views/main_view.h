#pragma once

#include "view_manager.h"

#include <iostream>
#include <fstream>
#include <streambuf>

#include "lua/lua.hpp"

#include "vm/machine.h"
#include "vm/lua_bridge.h"

namespace ui
{
  enum class Scale
  {
    UNSCALED,
    SCALED_ASPECT_2x,
    FULLSCREEN
  };

  class GameView : public View
  {
  private:
    Scale scale = Scale::UNSCALED;

    ViewManager* manager;

    SDL_Surface* _output;
    SDL_Texture* _outputTexture;
    SDL_Texture* _font;

    std::string _path;

    void rasterize();
    void render();
    void update();

  public:
    GameView(ViewManager* manager);
    ~GameView();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

    void loadCartridge(const std::string& path) { _path = path; }

    void text(const std::string& text, int32_t x, int32_t y);
  };
}
