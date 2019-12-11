#pragma once

#include "sdl_helper.h"

#include <array>

namespace ui
{
  class View
  {
  public:
    virtual void render() = 0;
    virtual void handleKeyboardEvent(const SDL_Event& event) = 0;
    virtual void handleMouseEvent(const SDL_Event& event) = 0;
  };

  struct ButtonStyle
  {
    bool pressed;
    bool hovered;
  };

  class GameView;

  class ViewManager : public SDL<ViewManager, ViewManager>
  {
  public:
    using view_t = View;
    static const size_t VIEW_COUNT = 1;

  private:
    std::array<view_t*, 2> views;
    view_t* view;

  public:
    ViewManager();

    bool loadData();

    void handleKeyboardEvent(const SDL_Event& event, bool press);
    void handleMouseEvent(const SDL_Event& event);
    void render();

    void deinit();

    //TODO: hacky cast to avoid header inclusion
    GameView* gameView() { return (GameView*)views[0]; }
  };
}

