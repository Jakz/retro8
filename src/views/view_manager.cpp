#include "view_manager.h"

#include "main_view.h"
#include "gen/pico_font.h"

#ifdef _WIN32
#define PREFIX  "../../../"
#else
#define PREFIX ""
#endif

using namespace ui;

extern retro8::Machine machine;

ui::ViewManager::ViewManager() : SDL<ui::ViewManager, ui::ViewManager>(*this, *this), _font(),
_gameView(new GameView(this)), _menuView(new MenuView(this))
{
  _view = _gameView;
}

void ui::ViewManager::deinit()
{
  _font.release();
  SDL::deinit();
}

bool ui::ViewManager::loadData()
{
  {
    constexpr size_t FONT_WIDTH = 128, FONT_HEIGHT = 80;

    /* create texture for font 128x80 1bit per pixel font */
    const SDL_PixelFormat* format = displayFormat();

    _font = allocate(FONT_WIDTH, FONT_HEIGHT);
    
    for (size_t i = 0; i < FONT_WIDTH*FONT_HEIGHT; ++i)
      _font.pixel(i) = (retro8::gfx::font_map[i / 8] & (1 << (7 - (i % 8)))) ? 0xffffffff : 0;

#if !defined(SDL12)
    _font.texture = SDL_CreateTextureFromSurface(renderer(), _font.surface);
#endif

    _font.enableBlending();
    _font.releaseSurface();
  }

  machine.font().load();

  return true;
}

void ui::ViewManager::handleKeyboardEvent(const SDL_Event& event)
{
  _view->handleKeyboardEvent(event);
}

void ui::ViewManager::handleMouseEvent(const SDL_Event& event)
{
  _view->handleMouseEvent(event);
}


void ui::ViewManager::render()
{
  _view->render();
}

void ui::ViewManager::text(const std::string& text, int32_t x, int32_t y)
{
  constexpr float scale = 2.0;
  constexpr int32_t GLYPHS_PER_ROW = 16;

  for (size_t i = 0; i < text.length(); ++i)
  {
    SDL_Rect src = SDL_MakeRect(8 * (text[i] % GLYPHS_PER_ROW), 8 * (text[i] / GLYPHS_PER_ROW), 4, 6);
    SDL_Rect dest = SDL_MakeRect(x + 4 * i * scale, y, 4 * scale, 6 * scale);
    blit(_font, src, dest);
  }
}

void ViewManager::text(const std::string& text, int32_t x, int32_t y, SDL_Color color, TextAlign align, float scale)
{
  constexpr int32_t GLYPHS_PER_ROW = 16;

  const int32_t width = text.size() * 4 * scale;

  if (align == TextAlign::CENTER)
    x -= width / 2;
  else if (align == TextAlign::RIGHT)
    x -= width;

#if !defined(SDL12)
  SDL_SetTextureColorMod(_font.texture, color.r, color.g, color.b);
#endif

  for (size_t i = 0; i < text.length(); ++i)
  {
    SDL_Rect src = SDL_MakeRect(8 * (text[i] % GLYPHS_PER_ROW), 8 * (text[i] / GLYPHS_PER_ROW), 4, 6);
    SDL_Rect dest = SDL_MakeRect(x + 4 * i * scale, y, 4 * scale, 6 * scale);
    blit(_font, src, dest);
  }

#if !defined(SDL12)
  SDL_SetTextureColorMod(_font.texture, 255, 255, 255);
#endif
}

void ViewManager::openMenu()
{
  _gameView->pause();
  _menuView->reset();
  _view = _menuView;
}

void ViewManager::backToGame()
{
  _gameView->resume();
  _view = _gameView;
}

void ViewManager::setPngCartridge(SDL_Surface* cartridge)
{
  _menuView->setPngCartridge(cartridge);
}
