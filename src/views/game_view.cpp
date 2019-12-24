#include "main_view.h"

#include "io/loader.h"

#include <future>

using namespace ui;
namespace r8 = retro8;


retro8::Machine machine;


GameView::GameView(ViewManager* manager) : manager(manager),
_paused(false), _showFPS(false), _showCartridgeName(false)
{
}


void GameView::update()
{
  machine.code().update();
  machine.code().draw();
}

void rasterize()
{

}

bool init = false;
void GameView::render()
{
  if (!init)
  {
    SDL_RendererInfo info;
    SDL_GetRendererInfo(manager->renderer(), &info);
    //SDL_PixelFormat* format = SDL_AllocFormat(SDL_GetWindowPixelFormat(manager->window()));

    SDL_PixelFormat* format = SDL_AllocFormat(info.texture_formats[0]);

    /* initialize color table to current pixel format */
    r8::gfx::ColorTable::init(format);

    printf("Using renderer pixel format: %s\n", SDL_GetPixelFormatName(format->format));

    /* initialize main surface and its texture */
    _output = SDL_CreateRGBSurface(0, 128, 128, 32, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    _outputTexture = SDL_CreateTexture(manager->renderer(), format->format, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    if (!_output)
    {
      printf("Unable to allocate buffer surface: %s\n", SDL_GetError());
    }

    assert(_outputTexture);
    assert(_output);

    for (uint32_t i = 0; i < keyStatus.size(); ++i)
    {
      keyStatus[i].button = retro8::button_t(1 << i);
      keyStatus[i].state = KeyStatus::State::OFF;
    }

    _frameCounter = 0;

    //SDL_FreeFormat(format);

    // save previous button state for btnp function
    machine.init(_output);

    namespace r8 = retro8;

    SDL_Surface* font = IMG_Load("pico8_font.png");
    machine.font().load(font);
    _font = SDL_CreateTextureFromSurface(manager->renderer(), font);
    SDL_FreeSurface(font);

    machine.code().loadAPI();

    retro8::io::Loader loader;

    if (_path.empty())
      _path = "cartridges/pico-racer.png";

    loader.load(_path, machine);
    machine.memory().backupCartridge();

    int32_t fps = machine.code().require60fps() ? 60 : 30;
    manager->setFrameRate(fps);

    if (machine.code().hasInit())
    {
      /* init is launched on a different thread because some developers are using busy loops and manual flips */
      _initFuture = std::async(std::launch::async, []() {
        LOGD("Cartridge has _init() function, calling it.");
        machine.code().init();
        LOGD("_init() function completed execution.");
      });
    }

    machine.sound().init();
    machine.sound().resume();

    init = true;
  }

  manageKeyRepeat();

  auto* renderer = manager->renderer();

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  if (!_paused)
  {
    if (!_initFuture.valid() || _initFuture.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready)
    {
      update();
      machine.flip();
    }

    SDL_UpdateTexture(_outputTexture, nullptr, _output->pixels, _output->pitch);
  }

SDL_Rect dest;
  //SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, machine.screen());
#ifdef _WIN32
  dest = { (480 - 384) / 2, (480 - 384) / 2, 384, 384 };
#else

if (scale == Scale::UNSCALED)
  dest = { (320 - 128) / 2, (240 - 128) / 2, 128, 128 };
else if (scale == Scale::SCALED_ASPECT_2x)
  dest = { (320 - 256) / 2, (240 - 256) / 2, 256, 256 };
else
  dest = { 0, 0, 320, 240 };
#endif

  SDL_RenderCopy(renderer, _outputTexture, nullptr, &dest);

  text(_path.c_str(), 10, 10);
  char buffer[16];
  sprintf(buffer, "%.0f/%c0", 1000.0f / manager->lastFrameTicks(), machine.code().require60fps() ? '6' : '3');
  text(buffer, 10, 22);

  ++_frameCounter;

#if DEBUGGER
  {
    /* sprite sheet */
    {
      SDL_Surface* spritesheet = SDL_CreateRGBSurface(0, 128, 128, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

      SDL_FillRect(spritesheet, nullptr, 0xFFFFFFFF);
      auto* dest = static_cast<uint32_t*>(spritesheet->pixels);
      for (r8::coord_t y = 0; y < r8::gfx::SPRITE_SHEET_HEIGHT; ++y)
        for (r8::coord_t x = 0; x < r8::gfx::SPRITE_SHEET_PITCH; ++x)
        {
          const r8::gfx::color_byte_t* data = machine.memory().as<r8::gfx::color_byte_t>(r8::address::SPRITE_SHEET + y * r8::gfx::SPRITE_SHEET_PITCH + x);
          RASTERIZE_PIXEL_PAIR(machine, dest, data);
        }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, spritesheet);
      SDL_Rect destr = { (1024 - 286) , 30, 256, 256 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(spritesheet);
    }

    /* palettes */
    {
      SDL_Surface* palettes = SDL_CreateRGBSurface(0, 16, 2, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

      SDL_FillRect(palettes, nullptr, 0xFFFFFFFF);
      auto* dest = static_cast<uint32_t*>(palettes->pixels);

      for (r8::palette_index_t j = 0; j < 2; ++j)
      {
        const r8::gfx::palette_t* palette = machine.memory().paletteAt(j);

        for (size_t i = 0; i < r8::gfx::COLOR_COUNT; ++i)
          dest[j*16 + i] = r8::gfx::ColorTable::get(palette->get(r8::color_t(i)));
      }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, palettes);
      SDL_Rect destr = { (1024 - 286) , 300, 256, 32 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(palettes);
    }


    {
      /*static SDL_Surface* tilemap = nullptr;

      if (!tilemap)
      {
        tilemap = SDL_CreateRGBSurface(0, 1024, 512, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
        SDL_FillRect(tilemap, nullptr, 0x00000000);
        uint32_t* base = static_cast<uint32_t*>(tilemap->pixels);
        for (r8::coord_t ty = 0; ty < r8::gfx::TILE_MAP_HEIGHT; ++ty)
        {
          for (r8::coord_t tx = 0; tx < r8::gfx::TILE_MAP_WIDTH; ++tx)
          {
            r8::sprite_index_t index = *machine.memory().spriteInTileMap(tx, ty);

            for (r8::coord_t y = 0; y < r8::gfx::SPRITE_HEIGHT; ++y)
              for (r8::coord_t x = 0; x < r8::gfx::SPRITE_WIDTH; ++x)
              {
                auto* dest = base + x + tx * r8::gfx::SPRITE_WIDTH + (y + ty * r8::gfx::SPRITE_HEIGHT) * tilemap->h;
                const r8::gfx::color_byte_t& pixels = machine.memory().spriteAt(index)->byteAt(x, y);
                RASTERIZE_PIXEL_PAIR(machine, dest, &pixels);
              }
          }
        }
      }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tilemap);
      SDL_Rect destr = { (1024 - 286) , 256, 256, 128 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      //SDL_FreeSurface(tilemap);*/
    }

  }
#endif
}

void GameView::text(const std::string& text, int32_t x, int32_t y)
{
  constexpr float scale = 2.0;
  constexpr int32_t GLYPHS_PER_ROW = 16;

  for (size_t i = 0; i < text.length(); ++i)
  {
    SDL_Rect src = { 8 * (text[i] % GLYPHS_PER_ROW), 8 * (text[i] / GLYPHS_PER_ROW), 4, 6 };
    SDL_Rect dest = { x + 4 * i * scale, y, 4 * scale, 6 * scale };
    SDL_RenderCopy(manager->renderer(), _font, &src, &dest);
  }
}

void GameView::manageKeyRepeat()
{
  static constexpr uint32_t TICKS_FOR_FIRST_REPEAT = 15;
  static constexpr uint32_t TICKS_REPEATING = 4;

  /* manage key repeats */
  const uint32_t ticks = _frameCounter;
  for (KeyStatus& ks : keyStatus)
  {
    if (ks.state == KeyStatus::State::FIRST)
    {
      machine.state().previousButtons.set(ks.button);
      ks.state = KeyStatus::State::WAITING;
    }
    else if (ks.state == KeyStatus::State::WAITING && (ticks - ks.ticks) >= TICKS_FOR_FIRST_REPEAT)
    {
      machine.state().previousButtons.set(ks.button);
      ks.state = KeyStatus::State::REPEATING;
      ks.ticks = ticks;
    }
    else if (ks.state == KeyStatus::State::REPEATING && (ticks - ks.ticks) >= TICKS_REPEATING)
    {
      machine.state().previousButtons.set(ks.button);
      ks.ticks = ticks;
    }
    else
      machine.state().previousButtons.reset(ks.button);
  }
}

void GameView::manageKey(size_t index, bool pressed)
{
  machine.state().buttons.set(keyStatus[index].button, pressed);
  keyStatus[index].ticks = _frameCounter;
  keyStatus[index].state = pressed ? KeyStatus::State::FIRST : KeyStatus::State::OFF;
}

void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  if (!event.key.repeat)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_LEFT:
      manageKey(0, event.type == SDL_KEYDOWN);
      break;
    case SDLK_RIGHT:
      manageKey(1, event.type == SDL_KEYDOWN);
      break;
    case SDLK_UP:
      manageKey(2, event.type == SDL_KEYDOWN);
      break;
    case SDLK_DOWN:
      manageKey(3, event.type == SDL_KEYDOWN);
      break;

    case SDLK_z:
    case SDLK_LCTRL:
      manageKey(4, event.type == SDL_KEYDOWN);
      break;

    case SDLK_x:
    case SDLK_LALT:
      manageKey(5, event.type == SDL_KEYDOWN);
      break;

    case SDLK_p:
      if (event.type == SDL_KEYDOWN)
        _paused = !_paused;

#if SOUND_ENABLED
      if (_paused)
        machine.sound().pause();
      else
        machine.sound().resume();
#endif
      break;


#ifndef _WIN32
    case SDLK_TAB:
      if (event.type == SDL_KEYDOWN)
      {
        if (scale == Scale::UNSCALED) scale = Scale::SCALED_ASPECT_2x;
        else if (scale == Scale::SCALED_ASPECT_2x) scale = Scale::FULLSCREEN;
        else scale = Scale::UNSCALED;
    }
      break;
#endif

    case SDLK_ESCAPE:
      if (event.type == SDL_KEYDOWN)
        manager->exit();
      break;
  }
  }
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}

GameView::~GameView()
{
  SDL_FreeSurface(_output);
  SDL_DestroyTexture(_outputTexture);

  machine.sound().close();
}
