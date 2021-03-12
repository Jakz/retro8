#if defined(SDL12)

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    printf("Error on SDL_Init().\n");
    return false;
  }

  SDL_EnableKeyRepeat(0, 0);

#if defined(WINDOW_SCALE)
  _screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
  #else
  _screen = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE);
#endif

  _format = _screen->format;
  SDL_WM_SetCaption("retro-8", nullptr);

  return true;
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::loop()
{
  while (!willQuit)
  {
    loopRenderer.render();
    SDL_Flip(_screen);

    handleEvents();

    capFPS();
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  SDL_FreeSurface(_screen);
  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_BlitSurface(texture, &from, _screen, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_BlitSurface(texture, from, _screen, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(texture, { sx, sy, w, h }, dx, dy);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, const SDL_Rect& src, const SDL_Rect& dest)
{
  SDL_BlitSurface(texture, const_cast<SDL_Rect*>(&src), _screen, const_cast<SDL_Rect*>(&dest));
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(Texture* texture, int dx, int dy)
{
  blit(texture, 0, 0, texture->w, texture->h, 0, 0, texture->w, texture->h);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::clear(int r, int g, int b)
{
  SDL_Rect to = { 0, 0, _screen->w, _screen->h };
  SDL_FillRect(_screen, &to, SDL_MapRGB(_screen->format, r, g, b));
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::rect(int x, int y, int w, int h, int r, int g, int b, int a)
{
  //TODO: implement
  /*SDL_SetRenderDrawColor(_renderer, r, g, b, a);
  SDL_Rect border = { x, y, w, h };
  SDL_RenderDrawRect(_renderer, &border);*/
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::release(Texture* texture)
{
  SDL_FreeSurface(texture);
}


inline static SDL_Rect SDL_MakeRect(int x, int y, int w, int h) { return { (Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h }; }

#endif