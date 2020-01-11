#include "main_view.h"

extern retro8::Machine machine;

using namespace ui;

struct MenuEntry
{
  mutable std::string caption;
  mutable std::function<void(void)> lambda;

  MenuEntry(const std::string& caption) : caption(caption) { }
};

static const std::vector<MenuEntry> mainMenu = {
  MenuEntry("resume"),
  MenuEntry("help"),
  MenuEntry("options"),
  MenuEntry("reset"),
  MenuEntry("exit")
};

static const std::vector<MenuEntry> optionsMenu = {
  MenuEntry("show fps"),
  MenuEntry("scaler 1:1"),
  MenuEntry("sound on"),
  MenuEntry("music on"),
  MenuEntry("back")
};

const std::vector<MenuEntry>* menu;
std::vector<MenuEntry>::const_iterator selected;

enum { RESUME = 0, HELP, OPTIONS, RESET, EXIT, SHOW_FPS = 0, SCALER, SOUND, MUSIC, BACK };

MenuView::MenuView(ViewManager* gvm) : _gvm(gvm), _cartridge(nullptr)
{
  static_assert(SCALER == 1, "must be 1");

  mainMenu[RESUME].lambda = [this]() {
    _gvm->backToGame();
  };

  mainMenu[OPTIONS].lambda = [this]() {
    menu = &optionsMenu;
    selected = menu->begin();
  };

  mainMenu[EXIT].lambda = [this]() {
    _gvm->exit();
  };

  optionsMenu[SHOW_FPS].lambda = [this]() {
    bool v = !_gvm->gameView()->isFPSShown();
    _gvm->gameView()->toggleFPS(v);
    updateLabels();
  };
  optionsMenu[SCALER].lambda = [this]() {
    auto scaler = _gvm->gameView()->scaler();
    if (scaler < Scaler::LAST)
      _gvm->gameView()->setScaler(Scaler(scaler + 1));
    else
      _gvm->gameView()->setScaler(Scaler::FIRST);
    updateLabels();
  };

  optionsMenu[SOUND].lambda = [this]() {
    bool v = !machine.sound().isSoundEnabled();
    machine.sound().toggleSound(v);
    updateLabels();
  };


  optionsMenu[MUSIC].lambda = [this]() {
    bool v = !machine.sound().isMusicEnabled();
    machine.sound().toggleMusic(v);
    updateLabels();
  };

  optionsMenu[BACK].lambda = [this]() {
    menu = &mainMenu;
    selected = menu->begin();
  };

  reset();
}

MenuView::~MenuView() { }

void MenuView::handleKeyboardEvent(const SDL_Event& event)
{
  static constexpr auto ACTION_BUTTON = SDLK_LCTRL;
  static constexpr auto BACK_BUTTON = SDLK_LALT;

  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_UP: if (selected > menu->begin()) --selected; else selected = menu->end() - 1; break;
    case SDLK_DOWN: if (selected < menu->end() - 1) ++selected; else selected = menu->begin(); break;
    case ACTION_BUTTON:
    {
      if (selected->lambda)
        selected->lambda();
      break;
    }
    case BACK_BUTTON:
    {
      if (menu == &mainMenu)
        mainMenu[0].lambda();
      else if (menu == &optionsMenu)
        optionsMenu[4].lambda();
      break;
    }
    }
  }
}

void MenuView::handleMouseEvent(const SDL_Event& event)
{

}

void MenuView::updateLabels()
{
  optionsMenu[MUSIC].caption = std::string("music ") + (machine.sound().isMusicEnabled() ? "on" : "off");
  optionsMenu[SOUND].caption = std::string("sound ") + (machine.sound().isSoundEnabled() ? "on" : "off");
  optionsMenu[SHOW_FPS].caption = std::string("show fps ") + (_gvm->gameView()->isFPSShown() ? "on" : "off");

  auto scaler = _gvm->gameView()->scaler();
  std::string scalerLabel = "scaler ";
  switch (scaler) {
  case Scaler::UNSCALED: scalerLabel += "1:1"; break;
  case Scaler::SCALED_ASPECT_2x: scalerLabel += "2:1"; break;
  case Scaler::FULLSCREEN: scalerLabel += "fit screen"; break;
  }

  optionsMenu[SCALER].caption = scalerLabel;
}

void MenuView::reset()
{
  menu = &mainMenu;
  selected = menu->begin();

  updateLabels();
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

  bool hasCartridge = _cartridge;

  // 160x205

  if (hasCartridge)
    _gvm->blit(_cartridge, W/2 - 5, (240 - 205)/2);

  const int32_t bx = hasCartridge ? W / 4 : W / 2;
  const int32_t by = 24;

  _gvm->text("retro8", bx + 2, by + 2, { 0, 22, 120 }, TextAlign::CENTER, 4.0f);
  _gvm->text("retro8", bx, by, { 0, 47, 255 }, TextAlign::CENTER, 4.0f);
  _gvm->text("v0.1b", bx + _gvm->textWidth("retro8", 4.0)/2 - _gvm->textWidth("v0.1b", 1.0), by + 24, { 0, 47, 255 }, TextAlign::LEFT, 1.0f);

  retro8::point_t menuBase = { bx, by + 70 };

  for (auto it = menu->begin(); it != menu->end(); ++it)
  {
    SDL_Color color = it == selected ? SDL_Color{ 255, 255, 0 } : SDL_Color{ 255,255,255 };

    if (it != selected && !it->lambda)
      color = { 160, 160, 160 };

    _gvm->text(it->caption, menuBase.x, menuBase.y, color, TextAlign::CENTER, 2.0f);
    menuBase.y += 16;
  }
}

void MenuView::setPngCartridge(SDL_Surface* cartridge)
{
  if (_cartridge)
    SDL_DestroyTexture(_cartridge);

  if (cartridge)
    _cartridge = SDL_CreateTextureFromSurface(_gvm->renderer(), cartridge);
  else
    _cartridge = nullptr;   
}