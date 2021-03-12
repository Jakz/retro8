#pragma once

#include "sdl_helper.h"

#include <array>
#include <string>

namespace ui
{
  class View
  {
  public:
    virtual void render() = 0;
    virtual void handleKeyboardEvent(const SDL_Event& event) = 0;
    virtual void handleMouseEvent(const SDL_Event& event) = 0;
  };

  enum TextAlign
  {
    LEFT, CENTER, RIGHT
  };

  class GameView;
  class MenuView;

  class ViewManager : public SDL<ViewManager, ViewManager>
  {
  public:
    using view_t = View;
    static const size_t VIEW_COUNT = 1;

    Surface _font;

  private:
    GameView* _gameView;
    MenuView* _menuView;
    view_t* _view;

  public:
    ViewManager();

    bool loadData();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);
    void render();

    void deinit();

    const Surface& font() { return _font; }

    //TODO: hacky cast to avoid header inclusion
    GameView* gameView() { return _gameView; }

    int32_t textWidth(const std::string& text, float scale = 2.0f) const { return text.length() * scale * 4; }
    void text(const std::string& text, int32_t x, int32_t y, SDL_Color color, TextAlign align, float scale = 2.0f);
    void text(const std::string& text, int32_t x, int32_t y);

    void openMenu();
    void backToGame();

    void setPngCartridge(SDL_Surface* cartridge);
  };
}

