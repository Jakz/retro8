#pragma once

#include "view_manager.h"


#include <lua.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "vm/machine.h"
#include "vm/lua_bridge.h"

#include "io/loader.h"

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
  };
}
