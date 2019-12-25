#include "main_view.h"

using namespace ui;

struct MenuEntry
{
  enum class Type
  {
    SUB_MENU,
    ENTRY
  };

  Type type;
  std::string caption;
  mutable std::function<void(void)> lambda;

  MenuEntry(const std::string& caption) : type(Type::ENTRY), caption(caption) { }
};

static const std::vector<MenuEntry> mainMenu = {
  MenuEntry("resume"),
  MenuEntry("help"),
  MenuEntry("options"),
  MenuEntry("reset"),
  MenuEntry("exit")
};

const std::vector<MenuEntry>* menu = &mainMenu;
std::vector<MenuEntry>::const_iterator selected = menu->begin();

MenuView::MenuView(ViewManager* gvm) : _gvm(gvm)
{ 
  mainMenu[4].lambda = [this]() {
    _gvm->exit();
  };
}

MenuView::~MenuView() { }

void MenuView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_UP: if (selected > menu->begin()) --selected; else selected = menu->end() - 1; break;
    case SDLK_DOWN: if (selected < menu->end() - 1) ++selected; else selected = menu->begin(); break;
    }
  }
}

void MenuView::handleMouseEvent(const SDL_Event& event)
{

}

void MenuView::render()
{
  constexpr int32_t W = 320;
  constexpr int32_t H = 240;

  auto renderer = _gvm->renderer();
  
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
  SDL_Rect border = { 0, 0, W, H };
  SDL_RenderDrawRect(renderer, &border);

  _gvm->text("retro8", W / 2 + 2, 20 + 2, { 0, 22, 120 }, TextAlign::CENTER, 4.0f);
  _gvm->text("retro8", W / 2, 20, { 0, 47, 255 }, TextAlign::CENTER, 4.0f);
  _gvm->text("v0.1", W / 2 + _gvm->textWidth("retro8", 4.0)/2 + 3, 34, { 0, 47, 255 }, TextAlign::LEFT, 2.0f);

  retro8::point_t menuBase = { W / 2, 80 };

  for (auto it = menu->begin(); it != menu->end(); ++it)
  {
    const SDL_Color color = it == selected ? SDL_Color{ 255, 255, 0 } : SDL_Color{ 255,255,255 };
    
    _gvm->text(it->caption, menuBase.x, menuBase.y, color, TextAlign::CENTER, 2.0f);
    menuBase.y += 16;
  }

}