#include "view_manager.h"


#include <lua.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "vm/machine.h"
#include "vm/lua_bridge.h"

#include "io/loader.h"

retro8::Machine machine;

namespace r8 = retro8;

namespace ui
{
  class GameView : public View
  {
  private:
    ViewManager* manager;

    void render();
    void update();

  public:
    GameView(ViewManager* manager);

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

  };

  GameView::GameView(ViewManager* manager) : manager(manager)
  {
  }

  void GameView::update()
  {
    machine.code().update();
    machine.code().draw();
  }

  bool init = false;
  void GameView::render()
  {
    if (!init)
    {
      // save previous button state for btnp function
      machine.state().previousButtons = machine.state().buttons;
      
      namespace r8 = retro8;

      machine.init();

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

      retro8::io::LoaderP8 loader;
      loader.load("demos/jelpi.p8", machine);

      manager->setFrameRate(machine.code().require60fps() ? 60 : 30);

      if (machine.code().hasInit())
        machine.code().init();

      /*for (int i = 0; i < 32; ++i)
        machine.circ(64, 64, i+1, (r8::color_t)(i % 15 + 1));*/
      
      //machine.circfill(64, 64, 13, r8::color_t::RED);
      //machine.circ(64, 64, 13, r8::color_t::GREEN);


      init = true;
    }

    auto* renderer = manager->getRenderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    update();
    machine.flip();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, machine.screen());
    SDL_Rect dest = { (640 - 384) / 2, (480 - 384) / 2, 384, 384 };
    //SDL_Rect dest = { (320 - 128) / 2, (240 - 128) / 2, 128, 128 };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);

#if DEBUGGER
    {
      SDL_Surface* surface = SDL_CreateRGBSurface(0, 128, 128, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
      SDL_FillRect(surface, nullptr, 0xFFFFFFFF);
      auto* dest = static_cast<uint32_t*>(surface->pixels);
      for (r8::coord_t y = 0; y < r8::gfx::SPRITE_SHEET_HEIGHT; ++y)
        for (r8::coord_t x = 0; x < r8::gfx::SPRITE_SHEET_WIDTH_IN_BYTES; ++x)
        {
          r8::gfx::color_byte_t* data = machine.memory().as<r8::gfx::color_byte_t>(r8::address::SPRITE_SHEET + y * r8::gfx::SPRITE_SHEET_WIDTH_IN_BYTES + x);

          const auto& rc1 = r8::gfx::ColorTable[data->low()];
          const auto& rc2 = r8::gfx::ColorTable[data->high()];

          *dest = (rc1.r << 16) | (rc1.g << 8) | (rc1.b) | 0xff000000;
          *(dest + 1) = (rc2.r << 16) | (rc2.g << 8) | (rc2.b) | 0xff000000;

          dest += 2;
        }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
      SDL_Rect destr = { (1024-286) , 30, 256, 256 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(surface);
    }
#endif

    SDL_DestroyTexture(texture);

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
      machine.state().buttons.set(retro8::button_t::ACTION1, event.type == SDL_KEYDOWN);
      break;

    case SDLK_x:
      machine.state().buttons.set(retro8::button_t::ACTION2, event.type == SDL_KEYDOWN);
      break;

      //TODO: missing action buttons

    case SDLK_ESCAPE:
      if (event.type == SDL_KEYDOWN)
        manager->exit();
      break;
 
    }
  }

  void GameView::handleMouseEvent(const SDL_Event& event)
  {

  }
}
