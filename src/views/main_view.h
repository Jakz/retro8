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
      c = machine.memory().penColor()->low();

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

    int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.memory().penColor()->low();

    machine.line(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

    return 0;
  }

  int rect(lua_State* L)
  {
    int x0 = lua_tointeger(L, 1);
    int y0 = lua_tointeger(L, 2);
    int x1 = lua_tointeger(L, 3);
    int y1 = lua_tointeger(L, 4);

    int c = lua_gettop(L) == 5 ? lua_tointeger(L, 5) : machine.memory().penColor()->low();

    machine.rect(x0, y0, x1, y1, static_cast<retro8::color_t>(c));

    return 0;
  }

  int cls(lua_State* L)
  {
    int c = lua_gettop(L) == 1 ? lua_tointeger(L, -1) : 0;

    machine.cls(static_cast<retro8::color_t>(c));

    return 0;
  }

  int spr(lua_State* L)
  {
    assert(lua_isnumber(L, 2) && lua_isnumber(L, 3));

    int idx = lua_tointeger(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);

    machine.spr(idx, x, y);

    return 0;
  }

  int pal(lua_State* L)
  {
    int c0 = lua_tonumber(L, 1);
    int c1 = lua_tonumber(L, 2);

    machine.pal(static_cast<retro8::color_t>(c0), static_cast<retro8::color_t>(c1));

    return 0;
  }

  static constexpr float PI = 3.14159265358979323846;


  int cos(lua_State* L)
  {
    assert(lua_isnumber(L, 1));

    float angle = lua_tonumber(L, 1);
    float value = cosf(angle * 2 * PI);
    lua_pushnumber(L, value);

    return 1;
  }

  int print(lua_State* L)
  {
    //TODO: optimize and use const char*?
    std::string text = lua_tostring(L, 1);
    int x = lua_tonumber(L, 2); //TODO: these are optional
    int y = lua_tonumber(L, 3);
    int c = lua_gettop(L) == 4 ? lua_tointeger(L, 4) : machine.memory().penColor()->low();

    machine.print(text, x, y, static_cast<retro8::color_t>(c));

    return 0;
  }

  class Code
  {
  private:
    lua_State* L;

  public:
    Code() : L(nullptr)
    {

    }

    ~Code()
    {
      if (L)
        lua_close(L);
    }

    void initFromSource(const std::string& code)
    {
      L = luaL_newstate();

      lua_register(L, "pset", pset);
      lua_register(L, "pget", pget);
      lua_register(L, "pal", pal);
      lua_register(L, "color", color);
      lua_register(L, "line", line);
      lua_register(L, "rect", rect);
      lua_register(L, "cls", cls);
      lua_register(L, "spr", spr);
      lua_register(L, "cos", cos);
      lua_register(L, "print", print);


      if (luaL_loadstring(L, code.c_str()))
      {
        const char* message = lua_tostring(L, -1);
        std::cout << "Error: on loadstring: " << message << std::endl;
      }

      int error = lua_pcall(L, 0, 0, 0);

      if (error)
      {
        const char* message = lua_tostring(L, -1);

        std::cout << "Error: script not loaded " << message << std::endl;
      }
    }

    void callVoidFunction(const char* name)
    {
      lua_getglobal(L, name);
      int error = lua_pcall(L, 0, 0, 0);

      if (error)
      {
        const char* message = lua_tostring(L, -1);

        std::cout << "Error in _draw function: " << message << std::endl;
      }
    }
  };
}



namespace ui
{
  class GameView : public View
  {
  private:
    ViewManager* manager;

    lua::Code code;

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
    code.callVoidFunction("_draw");
  }

  bool init = false;
  void GameView::render()
  {
    if (!init)
    {
      namespace r8 = retro8;

      machine.init();

      SDL_Surface* font = IMG_Load("pico8_font.png");
      machine.font().load(font);
      SDL_FreeSurface(font);

      SDL_Surface* surface = IMG_Load("hello_p8_gfx.png");
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

      SDL_FreeSurface(surface);

      const char* source =
        "t = 0\n"
        "\n"
        "function _draw()\n"
        "  cls(2)\n"
        "  for i=1,11 do\n"
        "    for j0=0,7 do\n"
        "      j = 7-j0\n"
        "      col = 7+j\n"
        "      t1 = t + i*4 - j*2\n"
        "      y = 38 + j + cos(t1/50)*5\n"
        "      pal(7,col)\n"
        "      spr(16+i, 8+i*8 + 5, y)\n"
        "    end\n"
        "  end\n"
        "  t = t + 1.0\n"
        "  print(\"this is pico-8\", 37, 70, 14)\n"
        "  print(\"nice to meet you\", 34, 80, 12)\n"
        "  spr(1, 64-4, 90)\n"
        "end\n"
      ;

      code.initFromSource(source);

      init = true;
    }

    auto* renderer = manager->getRenderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    update();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, machine.screen());
    SDL_Rect dest = { (640 - 384) / 2, (480 - 384) / 2, 384, 384 };
    //SDL_Rect dest = { (320 - 128) / 2, (240 - 128) / 2, 128, 128 };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);

  }

  void GameView::handleKeyboardEvent(const SDL_Event& event)
  {
    if (event.type == SDL_KEYDOWN)
    {
      switch (event.key.keysym.sym)
      {
        case SDLK_ESCAPE:
          manager->exit();
          break;
      }
    }
  }

  void GameView::handleMouseEvent(const SDL_Event& event)
  {

  }
}
