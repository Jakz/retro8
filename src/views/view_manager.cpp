#include "view_manager.h"

#include "main_view.h"

#ifdef _WIN32
#define PREFIX  "../../../"
#else
#define PREFIX ""
#endif

ui::ViewManager::ViewManager() : SDL<ui::ViewManager, ui::ViewManager>(*this, *this), textureUI(nullptr)
{
  views[0] = new GameView(this);
  view = views[0];
}

void ui::ViewManager::deinit()
{
  SDL_DestroyTexture(textureUI);
  SDL::deinit();
}

bool ui::ViewManager::loadData()
{
  SDL_Surface* surfaceUI = IMG_Load(PREFIX "data/ui.png");

  if (!surfaceUI)
  {
    printf("Error while loading ui.png: %s\n", IMG_GetError());
    return false;
  }

  textureUI = SDL_CreateTextureFromSurface(renderer, surfaceUI);
  SDL_FreeSurface(surfaceUI);

  return true;
}

bool pressed = false;

void ui::ViewManager::handleKeyboardEvent(const SDL_Event& event, bool press)
{
  view->handleKeyboardEvent(event);
}

void ui::ViewManager::handleMouseEvent(const SDL_Event& event)
{
  view->handleMouseEvent(event);
}


void ui::ViewManager::render()
{
  view->render();
}