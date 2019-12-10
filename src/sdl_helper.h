#pragma once

#include "common.h"

#include "SDL.h"
#include "SDL_image.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

enum class Align { LEFT, CENTER, RIGHT };

template<typename EventHandler, typename Renderer>
class SDL
{
protected:
  EventHandler& eventHandler;
  Renderer& loopRenderer;

  SDL_Window* window;
  SDL_Renderer* renderer;

#if defined(WINDOW_SCALE)
  SDL_Texture* buffer;
#endif

  bool willQuit;
  u32 ticks;

  u32 frameRate;
  float ticksPerFrame;


public:
  SDL(EventHandler& eventHandler, Renderer& loopRenderer) : eventHandler(eventHandler), loopRenderer(loopRenderer), 
    window(nullptr), renderer(nullptr), willQuit(false), ticks(0)
  {
    setFrameRate(60);
  }

  void setFrameRate(u32 frameRate)
  {
    this->frameRate = frameRate;
    this->ticksPerFrame = 1000 / (float)frameRate;
  }

  bool init();
  void deinit();
  void capFPS();
  
  void loop();
  void handleEvents();

  void exit() { willQuit = true; }

  void blit(SDL_Texture* texture, const SDL_Rect& src, int dx, int dy);
  void blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy);
  void blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
  void blit(SDL_Texture* texture, int dx, int dy);

  //void slowTextBlit(TTF_Font* font, int dx, int dy, Align align, const std::string& string);

  SDL_Renderer* getRenderer() { return renderer; }
};

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    printf("Error on SDL_Init().\n");
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    printf("Error on IMG_Init().\n");
    return false;
  }
  
  // SDL_WINDOW_FULLSCREEN
#if defined(WINDOW_SCALE)
#if defined(DEBUGGER)
  window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 480, SDL_WINDOW_OPENGL);
#else
  window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
#endif
#else
  window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_OPENGL);
#endif
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

#if defined(WINDOW_SCALE)
  buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
#endif

  return true;
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::loop()
{
  while (!willQuit)
  {
#if false && defined(WINDOW_SCALE)
    SDL_SetRenderTarget(renderer, buffer);
    loopRenderer.render();
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderCopy(renderer, buffer, nullptr, nullptr);
    SDL_RenderPresent(renderer);
#else
    loopRenderer.render();
    SDL_RenderPresent(renderer);
#endif

    handleEvents();

    capFPS();
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::capFPS()
{
  u32 ticks = SDL_GetTicks();
  u32 elapsed = ticks - SDL::ticks;

  //printf("Ticks: %u, waiting %f ticks, aticks: %u\n", elapsed, TICKS_PER_FRAME - elapsed, Gfx::fticks);

  u32 frameTime = elapsed;

  if (elapsed < ticksPerFrame)
  {
    SDL_Delay(ticksPerFrame - elapsed);
    frameTime = ticksPerFrame;
  }

  SDL::ticks = SDL_GetTicks();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  IMG_Quit();
  
#if defined(WINDOW_SCALE)
  SDL_DestroyTexture(buffer);
#endif

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  
  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      willQuit = true;
      break;

    case SDL_KEYDOWN:
      eventHandler.handleKeyboardEvent(event, true);
      break;

    case SDL_KEYUP:
      eventHandler.handleKeyboardEvent(event, false);
      break;

#if MOUSE_ENABLED
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
#if defined(WINDOW_SCALE)
      event.button.x /= WINDOW_SCALE;
      event.button.y /= WINDOW_SCALE;
#endif
      eventHandler.handleMouseEvent(event);
#endif
    }
  }
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_RenderCopy(renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_RenderCopy(renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(texture, { sx, sy, w, h }, dx, dy);
}


template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(texture, &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(renderer, texture, &from, &to);
}