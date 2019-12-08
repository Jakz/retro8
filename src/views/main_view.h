#include "view_manager.h"


#include <lua.hpp>
#include <iostream>

#include "vm/machine.h"

retro8::Machine machine;

namespace lua
{
  int pset(lua_State* L)
  {
    int args = lua_gettop(L);
    //TODO: check validity of arguments

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int c;

    if (args == 3)
      c = lua_tointeger(L, 3);
    else
      c = machine.state().penColor;

    machine.pset(x, y, static_cast<retro8::color_t>(c));

    return 0;
  }

  int pget(lua_State* L)
  {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);

    lua_pushinteger(L, machine.pget(x, y));

    return 1;
  }

  int color(lua_State* L)
  {
    int c = lua_tointeger(L, 1);

    machine.color(static_cast<retro8::color_t>(c));

    return 0;
  }

  int line(lua_State* L)
  {
    int x0 = lua_tointeger(L, 1);
    int y0 = lua_tointeger(L, 2);
    int x1 = lua_tointeger(L, 3);
    int y1 = lua_tointeger(L, 4);

    int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.state().penColor;

    machine.line(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

    return 0;
  }

  int rect(lua_State* L)
  {
    int x0 = lua_tointeger(L, 1);
    int y0 = lua_tointeger(L, 2);
    int x1 = lua_tointeger(L, 3);
    int y1 = lua_tointeger(L, 4);

    int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.state().penColor;

    machine.rect(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

    return 0;
  }
  
  class Script
  {
  private:
    lua_State* L;

  public:
    Script(const std::string& code) : L(nullptr)
    {
      L = luaL_newstate();

      lua_register(L, "pset", pset);
      lua_register(L, "pget", pget);
      lua_register(L, "color", color);
      lua_register(L, "line", line);
      lua_register(L, "rect", rect);

      if (!luaL_loadstring(L, code.c_str()))
      {
        std::cout << "Error: on loadstring" << std::endl;
      }

      int error = lua_pcall(L, 0, 0, 0);

      if (error)
      {
        const char* message = lua_tostring(L, -1);

        std::cout << "Error: script not loaded " << message << std::endl;
      }
    }

    ~Script()
    {
      if (L)
        lua_close(L);
    }
  };
}



namespace ui
{
  class GameView : public View
  {
  private:
    ViewManager* manager;

  public:
    GameView(ViewManager* manager);

    void render();
    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);

  };

  GameView::GameView(ViewManager* manager) : manager(manager)
  { 
    namespace r8 = retro8;
    
    SDL_Surface* surface = IMG_Load("hello_p8_gfx.png");
    assert(surface);

    for (size_t s = 0; s < 16; ++s)
    {
      retro8::gfx::sprite_t* sprite = machine.memory().spriteAt(s);

      for (retro8::coord_t y = 0; y < retro8::gfx::SPRITE_HEIGHT; ++y)
        for (retro8::coord_t x = 0; x < retro8::gfx::SPRITE_WIDTH; ++x)
        {
          SDL_Color color;

          if (SDL_ISPIXELFORMAT_INDEXED(surface->format->format))
          {
            color = surface->format->palette->colors[((uint8_t*)surface->pixels)[(y*surface->w) + s*8 + x]];
          }

          
          retro8::color_t r8color = retro8::gfx::colorForRGB((color.r << 16 | color.g << 8 | color.b));
          sprite->set(x, y, r8color);
        }
    }


    
    /*lua::Script script = lua::Script(
      "pset(10, 10, 11)\n"
      "color(8)\n"
      "pset(20, 20)\n"
      "pset(15, 15, pget(10, 10))\n"
      "color(3)\n"
      "rect(50, 50, 65, 78, 2)"
    );*/

    for (size_t i = 0; i < 16; ++i)
      machine.spr(1, 0 * 16, 90);

  }

  void GameView::render()
  {
    auto* renderer = manager->getRenderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, machine.screen());
    SDL_Rect dest = { (640 - 384) / 2, (480 - 384) / 2, 384, 384 };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);

  }

  void GameView::handleKeyboardEvent(const SDL_Event& event)
  {

  }

  void GameView::handleMouseEvent(const SDL_Event& event)
  {

  }
}
