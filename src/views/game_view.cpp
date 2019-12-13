#include "main_view.h"

using namespace ui;
namespace r8 = retro8;

retro8::Machine machine;


GameView::GameView(ViewManager* manager) : manager(manager)
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

    printf("[SDL] Window pixel format: %s\n", SDL_GetPixelFormatName(format->format));

    /* initialize main surface and its texture */
    _output = SDL_CreateRGBSurface(0, 128, 128, 32, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    _outputTexture = SDL_CreateTexture(manager->renderer(), format->format, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    if (!_output)
    {
      printf("Unable to allocate buffer surface: %s\n", SDL_GetError());
    }

    assert(_outputTexture);
    assert(_output);

    //SDL_FreeFormat(format);

    // save previous button state for btnp function
    machine.state().previousButtons = machine.state().buttons;
    machine.init(_output);

    namespace r8 = retro8;

    SDL_Surface* font = IMG_Load("pico8_font.png");
    machine.font().load(font);
    SDL_FreeSurface(font);

    /*SDL_Surface* surface = IMG_Load("hello_p8_gfx.png");
    assert(surface);

    static constexpr size_t SPRITES_PER_ROW = 16;

    for (size_t s = 0; s < 32; ++s)
    {
      retro8::gfx::sprite_t* sprite = machine.memory().spriteAt(s);

      r8::coord_t bx = (s % SPRITES_PER_ROW) * r8::gfx::SPRITE_WIDTH;
      r8::coord_t by = (s / SPRITES_PER_ROW) * r8::gfx::SPRITE_HEIGHT;

      for (retro8::coord_t y = 0; y < retro8::gfx::SPRITE_HEIGHT; ++y)
        for (retro8::coord_t x = 0; x < retro8::gfx::SPRITE_WIDTH; ++x)
        {
          SDL_Color color;

          if (SDL_ISPIXELFORMAT_INDEXED(surface->format->format))
          {
            color = surface->format->palette->colors[((uint8_t*)surface->pixels)[((y + by) * surface->w) + bx + x]];
          }


          retro8::color_t r8color = retro8::gfx::colorForRGB((color.r << 16 | color.g << 8 | color.b));
          sprite->set(x, y, r8color);
        }
    }

    SDL_FreeSurface(surface);*/

    /*std::ifstream i("bounce.p8");
    std::string str((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>());

    code.initFromSource(str.c_str());*/

    machine.code().loadAPI();

    retro8::io::LoaderP8 loader;
    std::string path = !_path.empty() ? _path : "pico_physics.p8";
    loader.load(path, machine);

    manager->setFrameRate(machine.code().require60fps() ? 60 : 30);

    if (machine.code().hasInit())
      machine.code().init();

    /*for (int i = 0; i < 32; ++i)
      machine.circ(64, 64, i+1, (r8::color_t)(i % 15 + 1));*/

      //machine.circfill(64, 64, 13, r8::color_t::RED);
      //machine.circ(64, 64, 13, r8::color_t::GREEN);


    init = true;
  }

  auto* renderer = manager->renderer();

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  update();
  machine.flip();
  int r = SDL_UpdateTexture(_outputTexture, nullptr, _output->pixels, _output->pitch);

  if (r < 0)
  {
    printf("SDL Error: %s\n", SDL_GetError());
    assert(false);
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

#if DEBUGGER
  {
    /* sprite sheet */
    {
      SDL_Surface* spritesheet = SDL_CreateRGBSurface(0, 128, 128, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

      SDL_FillRect(spritesheet, nullptr, 0xFFFFFFFF);
      auto* dest = static_cast<uint32_t*>(spritesheet->pixels);
      for (r8::coord_t y = 0; y < r8::gfx::SPRITE_SHEET_HEIGHT; ++y)
        for (r8::coord_t x = 0; x < r8::gfx::SPRITE_SHEET_WIDTH_IN_BYTES; ++x)
        {
          const r8::gfx::color_byte_t* data = machine.memory().as<r8::gfx::color_byte_t>(r8::address::SPRITE_SHEET + y * r8::gfx::SPRITE_SHEET_WIDTH_IN_BYTES + x);
          RASTERIZE_PIXEL_PAIR(machine, dest, data);
        }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, spritesheet);
      SDL_Rect destr = { (1024 - 286) , 30, 256, 256 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(spritesheet);
    }

    /* palette */

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

void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  switch (event.key.keysym.sym)
  {
  case SDLK_LEFT:
    machine.state().buttons.set(retro8::button_t::LEFT, event.type == SDL_KEYDOWN);
    break;
  case SDLK_RIGHT:
    machine.state().buttons.set(retro8::button_t::RIGHT, event.type == SDL_KEYDOWN);
    break;
  case SDLK_UP:
    machine.state().buttons.set(retro8::button_t::UP, event.type == SDL_KEYDOWN);
    break;
  case SDLK_DOWN:
    machine.state().buttons.set(retro8::button_t::DOWN, event.type == SDL_KEYDOWN);
    break;

  case SDLK_z:
  case SDLK_LCTRL:
    machine.state().buttons.set(retro8::button_t::ACTION1, event.type == SDL_KEYDOWN);
    break;

  case SDLK_x:
  case SDLK_LALT:
    machine.state().buttons.set(retro8::button_t::ACTION2, event.type == SDL_KEYDOWN);
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

void GameView::handleMouseEvent(const SDL_Event& event)
{

}

GameView::~GameView()
{
  SDL_FreeSurface(_output);
  SDL_DestroyTexture(_outputTexture);
}
