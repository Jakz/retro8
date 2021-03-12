#pragma once

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    LOGD("Error on SDL_Init().\n");
    return false;
  }

  // SDL_WINDOW_FULLSCREEN
#if defined(WINDOW_SCALE)
#if defined(DEBUGGER)
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH*3, SCREEN_WIDTH*2, SDL_WINDOW_OPENGL);
#else
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_OPENGL);
#endif
#else
  _window = SDL_CreateWindow("retro-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
#endif
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  SDL_RendererInfo info;
  SDL_GetRendererInfo(_renderer, &info);
  _format = SDL_AllocFormat(info.texture_formats[0]);

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
void SDL<EventHandler, Renderer>::deinit()
{
  SDL_FreeFormat(_format);
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
Surface SDL<EventHandler, Renderer>::allocate(int width, int height)
{
  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, _format->Rmask, _format->Gmask, _format->Bmask, _format->Amask);
  SDL_Texture* texture = SDL_CreateTexture(_renderer, _format->format, SDL_TEXTUREACCESS_STREAMING, width, width);
  return { surface, texture };
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::blitToScreen(const Surface& surface, const SDL_Rect& rect)
{
  SDL_RenderCopy(_renderer, surface.texture, nullptr, &rect);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& src, const SDL_Rect& dest)
{
  SDL_RenderCopy(_renderer, surface.texture, &src, &dest);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(surface, { sx, sy, w, h }, dx, dy);
}


template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(surface.texture, &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(_renderer, surface.texture, &from, &to);
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

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::release(const Surface& surface)
{
  SDL_DestroyTexture(surface.texture);

  if (surface.surface)
    SDL_FreeSurface(surface.surface);
}

inline static SDL_Rect SDL_MakeRect(int x, int y, int w, int h) { return { x, y, w, h }; }

