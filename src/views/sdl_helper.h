#pragma once

#include "common.h"

#include "SDL.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

#if SDL_COMPILEDVERSION > 2000
using Texture = SDL_Texture;
#else
using Texture = SDL_Surface;
#endif

enum class Align { LEFT, CENTER, RIGHT };

template<typename EventHandler, typename Renderer>
class SDL
{
protected:
  EventHandler& eventHandler;
  Renderer& loopRenderer;

  SDL_Window* _window;
  SDL_Renderer* _renderer;

  bool willQuit;
  u32 ticks;
  float _lastFrameTicks;

  u32 frameRate;
  float ticksPerFrame;


public:
  SDL(EventHandler& eventHandler, Renderer& loopRenderer) : eventHandler(eventHandler), loopRenderer(loopRenderer),
    _window(nullptr), _renderer(nullptr), willQuit(false), ticks(0)
  {
    setFrameRate(60);
  }

  void setFrameRate(u32 frameRate)
  {
    this->frameRate = frameRate;
    this->ticksPerFrame = 1000 / (float)frameRate;
  }

  float lastFrameTicks() const { return _lastFrameTicks; }

  bool init();
  void deinit();
  void capFPS();

  void loop();
  void handleEvents();

  void exit() { willQuit = true; }

  void blit(Texture* texture, const SDL_Rect& src, int dx, int dy);
  void blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy);
  void blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
  void blit(Texture* texture, int dx, int dy);

  void clear(int r, int g, int b);
  void rect(int x, int y, int w, int h, int r, int g, int b, int a);

  //void slowTextBlit(TTF_Font* font, int dx, int dy, Align align, const std::string& string);

  SDL_Window* window() { return _window; }
  SDL_Renderer* renderer() { return _renderer; }
};

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    printf("Error on SDL_Init().\n");
    return false;
  }

  // SDL_WINDOW_FULLSCREEN
#if defined(WINDOW_SCALE)
#if defined(DEBUGGER)
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 480, SDL_WINDOW_OPENGL);
#else
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
#endif
#else
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_OPENGL);
#endif
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  return true;
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::loop()
{
  while (!willQuit)
  {
    loopRenderer.render();
    SDL_RenderPresent(_renderer);

    handleEvents();

    capFPS();
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::capFPS()
{
  u32 ticks = SDL_GetTicks();
  u32 elapsed = ticks - SDL::ticks;

  _lastFrameTicks = elapsed;

  if (elapsed < ticksPerFrame)
  {
    SDL_Delay(ticksPerFrame - elapsed);
    _lastFrameTicks = ticksPerFrame;
  }

  SDL::ticks = SDL_GetTicks();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

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
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(texture, { sx, sy, w, h }, dx, dy);
}


template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(texture, &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::clear(int r, int g, int b)
{
  SDL_SetRenderDrawColor(_renderer, r, g, b, 255);
  SDL_RenderClear(_renderer);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::rect(int x, int y, int w, int h, int r, int g, int b, int a)
{
  SDL_SetRenderDrawColor(_renderer, r, g, b, a);
  SDL_Rect border = { x, y, w, h };
  SDL_RenderDrawRect(_renderer, &border);
}

inline static SDL_Rect SDL_MakeRect(int x, int y, int w, int h) { return { x, y, w, h }; }

