#if defined(SDL12)

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    LOGD("Error on SDL_Init().\n");
    return false;
  }

  SDL_ShowCursor(0);
  SDL_EnableKeyRepeat(0, 0);

#if defined(WINDOW_SCALE)
  _screen = SDL_SetVideoMode(SCREEN_WIDTH*2, SCREEN_HEIGHT*2, 32, SDL_HWSURFACE);
  #else
  _screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
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

static constexpr auto RES_HW_SCREEN_VERTICAL = 240;
static constexpr auto RES_HW_SCREEN_HORIZONTAL = 240;

static void scale_NN_AllowOutOfScreen(SDL_Surface *src_surface, SDL_Surface *dst_surface, int new_w, int new_h)
{

  /// Sanity check
  if (src_surface->format->BytesPerPixel != dst_surface->format->BytesPerPixel) {
    printf("Error in %s, src_surface bpp: %d != dst_surface bpp: %d", __func__,
      src_surface->format->BytesPerPixel, dst_surface->format->BytesPerPixel);
    return;
  }

  int BytesPerPixel = src_surface->format->BytesPerPixel;
  int w1 = src_surface->w;
  //int h1=src_surface->h;
  int w2 = new_w;
  int h2 = new_h;
  int x_ratio = (int)((src_surface->w << 16) / w2);
  int y_ratio = (int)((src_surface->h << 16) / h2);
  int x2, y2;

  /// --- Compute padding for centering when out of bounds ---
  int y_padding = (RES_HW_SCREEN_VERTICAL - new_h) / 2;
  int x_padding = 0;
  if (w2 > RES_HW_SCREEN_HORIZONTAL) {
    x_padding = (w2 - RES_HW_SCREEN_HORIZONTAL) / 2 + 1;
  }
  int x_padding_ratio = x_padding * w1 / w2;
  //printf("src_surface->h=%d, h2=%d\n", src_surface->h, h2);

  for (int i = 0; i < h2; i++)
  {
    if (i >= RES_HW_SCREEN_VERTICAL) {
      continue;
    }

    uint8_t* t = (uint8_t*)(dst_surface->pixels) + ((i + y_padding) * ((w2 > RES_HW_SCREEN_HORIZONTAL) ? RES_HW_SCREEN_HORIZONTAL : w2))*BytesPerPixel;
    y2 = ((i*y_ratio) >> 16);
    uint8_t* p = (uint8_t*)(src_surface->pixels) + (y2*w1 + x_padding_ratio)*BytesPerPixel;
    int rat = 0;
    for (int j = 0; j < w2; j++)
    {
      if (j >= RES_HW_SCREEN_HORIZONTAL) {
        continue;
      }
      x2 = (rat >> 16);
      //*t++ = p[x2];
      memcpy(t, &p[x2*BytesPerPixel], BytesPerPixel);
      t += BytesPerPixel;
      rat += x_ratio;
    }
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::blitToScreen(const Surface& surface, const SDL_Rect& rect)
{
#if PLATFORM == PLATFORM_FUNKEY
  scale_NN_AllowOutOfScreen(surface.surface, _screen, 256, 256);
#else
  SDL_BlitSurface(surface.surface, nullptr, _screen, const_cast<SDL_Rect*>(&rect));
#endif
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  SDL_FreeSurface(_screen);
  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
Surface SDL<EventHandler, Renderer>::allocate(int width, int height)
{
  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, _format->Rmask, _format->Gmask, _format->Bmask, _format->Amask);
  return { surface };
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_BlitSurface(surface.surface, &from, _screen, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_BlitSurface(surface.surface, &from, _screen, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(surface, { sx, sy, w, h }, dx, dy);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, const SDL_Rect& src, const SDL_Rect& dest)
{
  SDL_BlitSurface(surface.surface, const_cast<SDL_Rect*>(&src), _screen, const_cast<SDL_Rect*>(&dest));
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(const Surface& surface, int dx, int dy)
{
  blit(surface.surface, 0, 0, surface.surface->w, surface.surface->h, 0, 0, surface.surface->w, surface.surface->h);
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
inline void SDL<EventHandler, Renderer>::release(const Surface& surface)
{
  SDL_FreeSurface(surface.surface);
}


inline static SDL_Rect SDL_MakeRect(int x, int y, int w, int h) { return { (Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h }; }

#endif
