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

ui::ViewManager::ViewManager() : SDL<ui::ViewManager, ui::ViewManager>(*this, *this), _font(nullptr),
_gameView(new GameView(this)), _menuView(new MenuView(this))
{
  _view = _gameView;
}

void ui::ViewManager::deinit()
{
  SDL_DestroyTexture(_font);

  SDL::deinit();
}

bool ui::ViewManager::loadData()
{
  {
    constexpr size_t FONT_WIDTH = 128, FONT_HEIGHT = 80;

    /* create texture for font
       128x80 1bit per pixel font
    */
    SDL_RendererInfo info;
    SDL_GetRendererInfo(_renderer, &info);
    SDL_PixelFormat* format = SDL_AllocFormat(info.texture_formats[0]);
    SDL_Surface* fontSurface = SDL_CreateRGBSurface(0, FONT_WIDTH, FONT_HEIGHT, 32, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    SDL_FreeFormat(format);

    auto pixels = static_cast<uint32_t*>(fontSurface->pixels);
    for (size_t i = 0; i < FONT_WIDTH*FONT_HEIGHT; ++i)
      pixels[i] = (retro8::gfx::font_map[i / 8] & (1 << (7 - (i % 8)))) ? 0xffffffff : 0;

    _font = SDL_CreateTextureFromSurface(_renderer, fontSurface);
    SDL_SetTextureBlendMode(_font, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(fontSurface);
  }

  machine.font().load();

  return true;
}

void ui::ViewManager::handleKeyboardEvent(const SDL_Event& event, bool press)
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
    SDL_RenderCopy(_renderer, _font, &src, &dest);
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

  SDL_SetTextureColorMod(_font, color.r, color.g, color.b);

  for (size_t i = 0; i < text.length(); ++i)
  {
    SDL_Rect src = SDL_MakeRect(8 * (text[i] % GLYPHS_PER_ROW), 8 * (text[i] / GLYPHS_PER_ROW), 4, 6);
    SDL_Rect dest = SDL_MakeRect(x + 4 * i * scale, y, 4 * scale, 6 * scale);
    SDL_RenderCopy(_renderer, _font, &src, &dest);
  }

  SDL_SetTextureColorMod(_font, 255, 255, 255);
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
