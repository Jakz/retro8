#pragma once

#include "common.h"

#include "SDL.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

#if SDL_COMPILEDVERSION > 2000
using Texture = SDL_Texture;

#if _WIN32
  #define DEBUGGER true
#endif

#else
using Texture = SDL_Surface;
#define SDL12

using SDL_Renderer = int;
using SDL_Window = int;
using SDL_AudioDeviceID = int;
#define SDL_OpenAudioDevice(x, y, w, s, z) SDL_OpenAudio(w, s)
#define SDL_PauseAudioDevice(_,y) SDL_PauseAudio(y)
#define SDL_CloseAudioDevice(_) SDL_CloseAudio()

#endif

enum class Align { LEFT, CENTER, RIGHT };

template<typename EventHandler, typename Renderer>
class SDL
{
protected:
  EventHandler& eventHandler;
  Renderer& loopRenderer;

  SDL_Surface* _screen;
  SDL_Window* _window;
  SDL_Renderer* _renderer;

  bool willQuit;
  u32 ticks;
  float _lastFrameTicks;

  u32 frameRate;
  float ticksPerFrame;


public:
  SDL(EventHandler& eventHandler, Renderer& loopRenderer) : eventHandler(eventHandler), loopRenderer(loopRenderer),
    _screen(nullptr), _window(nullptr), _renderer(nullptr), willQuit(false), ticks(0)
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

  void blit(Texture* texture, const SDL_Rect& src, const SDL_Rect& dest);
  void blit(Texture* texture, const SDL_Rect& src, int dx, int dy);
  void blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy);
  void blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
  void blit(Texture* texture, int dx, int dy);

  void clear(int r, int g, int b);
  void rect(int x, int y, int w, int h, int r, int g, int b, int a);

  void release(Texture* texture);

  SDL_Window* window() { return _window; }
  SDL_Renderer* renderer() { return _renderer; }
};

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

#if defined(SDL12)
#include "sdl_impl12.h"
#else
#include "sdl_impl.h"
#endif