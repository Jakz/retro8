#pragma once

#include "view_manager.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <future>

#include "lua/lua.hpp"

#include "vm/machine.h"
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

    SDL_Surface* _output;
    SDL_Texture* _outputTexture;

    std::string _path;

    std::future<void> _initFuture;

    bool _paused;

    bool _showFPS;
    bool _showCartridgeName;

    void render();
    void update();

    struct KeyStatus
    {
      enum class State { OFF, FIRST, WAITING, REPEATING } state;
      retro8::button_t button;
      uint32_t ticks;
    };
    std::array<KeyStatus, retro8::BUTTON_COUNT> keyStatus;

 
    void manageKeyRepeat();
    void manageKey(size_t index, bool pressed);

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

  public:
    MenuView(ViewManager* manager);
    ~MenuView();

    void handleKeyboardEvent(const SDL_Event& event) override;
    void handleMouseEvent(const SDL_Event& event) override;
    
    void render() override;

    void reset();
    void updateLabels();
  };
}
