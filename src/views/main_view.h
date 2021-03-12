#pragma once

#include "view_manager.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <future>

#include "lua/lua.hpp"

#include "vm/machine.h"
#include "vm/input.h"
#include "vm/lua_bridge.h"

namespace ui
{
  enum Scaler
  {
    UNSCALED = 0,
    SCALED_ASPECT_2x,
    FULLSCREEN,

    FIRST = UNSCALED,
    LAST = FULLSCREEN
  };

  class GameView : public View
  {
  private:
    uint32_t _frameCounter;
    Scaler _scaler = Scaler::UNSCALED;

    ViewManager* manager;

    retro8::input::InputManager _input;

    Surface _output;

    std::string _path;

    std::future<void> _initFuture;

    bool _paused;

    bool _showFPS;
    bool _showCartridgeName;

    void rasterize();
    void render();
    void update();

  public:
    GameView(ViewManager* manager);
    ~GameView();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

    void loadCartridge(const std::string& path) { _path = path; }

    void pause();
    void resume();

    void setScaler(Scaler scaler) { _scaler = scaler; }
    Scaler scaler() const { return _scaler; }

    void toggleFPS(bool active) { _showFPS = active; }
    bool isFPSShown() { return _showFPS; }
  };

  class MenuView : public View
  {
  private:
    ViewManager* _gvm;
    Surface _cartridge;

  public:
    MenuView(ViewManager* manager);
    ~MenuView();

    void handleKeyboardEvent(const SDL_Event& event) override;
    void handleMouseEvent(const SDL_Event& event) override;
    
    void render() override;

    void reset();
    void updateLabels();

    void setPngCartridge(SDL_Surface* cartridge);
  };
}
