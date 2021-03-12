#include "main_view.h"

#include "io/loader.h"
#include "io/stegano.h"

#include <future>

#include <SDL_audio.h>

class SDLAudio
{
private:
  SDL_AudioSpec spec;
  SDL_AudioDeviceID device;

  static void audio_callback(void* data, uint8_t* cbuffer, int length);

public:
  void init(retro8::sfx::APU* apu);

  void pause();
  void resume();
  void close();
};

void SDLAudio::audio_callback(void* data, uint8_t* cbuffer, int length)
{
  retro8::sfx::APU* apu = static_cast<retro8::sfx::APU*>(data);
  int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);
  apu->renderSounds(buffer, length / sizeof(int16_t));
  return;
}

void SDLAudio::init(retro8::sfx::APU* apu)
{
  SDL_AudioSpec wantSpec;
  wantSpec.freq = 44100;
  wantSpec.format = AUDIO_S16SYS;
  wantSpec.channels = 1;
  wantSpec.samples = 2048;
  wantSpec.userdata = apu;
  wantSpec.callback = audio_callback;

  device = SDL_OpenAudioDevice(NULL, 0, &wantSpec, &spec, 0);

  if (!device)
  {
    printf("Error while opening audio: %s", SDL_GetError());
  }
}

void SDLAudio::resume()
{
  SDL_PauseAudioDevice(device, false);
}

void SDLAudio::pause()
{
  SDL_PauseAudioDevice(device, true);
}

void SDLAudio::close()
{
  SDL_CloseAudioDevice(device);
}

SDLAudio sdlAudio;

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



retro8::io::PngData loadPng(const std::string& path)
{
  std::ifstream fl(path, std::ios_base::in | std::ios_base::binary);
  fl.seekg(0, std::ios::end);
  size_t length = fl.tellg();

  char* bdata = new char[length];
  
  fl.seekg(0, std::ios::beg);
  fl.read(bdata, length);

  fl.close();
  
  std::vector<uint8_t> out;
  unsigned long width, height;
  auto result = Platform::loadPNG(out, width, height, (uint8_t*)bdata, length, true);

  delete [] bdata;

  assert(result == 0);

  if (result != 0)
  {
    printf("Error while loading PNG cart.");
    assert(false);
  }

  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  std::memcpy(surface->pixels, out.data(), width*height * sizeof(uint32_t));

  retro8::io::PngData pngData = { static_cast<const uint32_t*>(surface->pixels), surface, static_cast<size_t>(surface->h * surface->w)};
  assert(surface->pitch == retro8::io::Stegano::IMAGE_WIDTH * sizeof(uint32_t));
  assert(surface->format->BytesPerPixel == 4);

  return pngData;
}

r8::gfx::ColorTable colorTable;

struct ColorMapper
{
  const SDL_PixelFormat* format;
  ColorMapper(const SDL_PixelFormat* format) : format(format) { }

  inline r8::gfx::ColorTable::pixel_t operator()(uint8_t r, uint8_t g, uint8_t b) const
  {
    return SDL_MapRGB(format, r, g, b);
  }
};

void GameView::rasterize()
{
  auto* data = machine.memory().screenData();
  auto* screenPalette = machine.memory().paletteAt(r8::gfx::SCREEN_PALETTE_INDEX);
  uint32_t* output = _output.pixels();

  for (size_t i = 0; i < r8::gfx::BYTES_PER_SCREEN; ++i)
  {
    const r8::gfx::color_byte_t* pixels = data + i;
    const auto rc1 = colorTable.get(screenPalette->get((pixels)->low()));
    const auto rc2 = colorTable.get(screenPalette->get((pixels)->high()));

    *(output) = rc1;
    *((output)+1) = rc2;
    (output) += 2;
  }
}



bool init = false;
void GameView::render()
{
  if (!init)
  {
    LOGD("Initializing color table");
    auto* format = manager->displayFormat();
    colorTable.init(ColorMapper(manager->displayFormat()));

#if !defined(SDL12)
    printf("Using renderer pixel format: %s\n", SDL_GetPixelFormatName(format->format));
#endif

    /* initialize main surface and its texture */
    _output = manager->allocate(128, 128);

    if (!_output)
    {
      printf("Unable to allocate buffer surface: %s\n", SDL_GetError());
    }

    assert(_output);

    _frameCounter = 0;

    machine.code().loadAPI();
    _input.setMachine(&machine);


    if (_path.empty())
      _path = "cartridges/pico-racer.png";

    if (r8::io::Loader::isPngCartridge(_path))
    {
      auto cartridge = loadPng(_path);

      retro8::io::Stegano stegano;
      stegano.load(cartridge, machine);

      manager->setPngCartridge(static_cast<SDL_Surface*>(cartridge.userData));
      SDL_FreeSurface(static_cast<SDL_Surface*>(cartridge.userData));
    }
    else
    {
      r8::io::Loader loader;
      loader.loadFile(_path, machine);
      manager->setPngCartridge(nullptr);
    }
    
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
    sdlAudio.init(&machine.sound());
    sdlAudio.resume();

    init = true;
  }

  _input.manageKeyRepeat();
  _input.tick();

  auto* renderer = manager->renderer();

  manager->clear(0, 0, 0);

  if (!_paused)
  {
    if (!_initFuture.valid() || _initFuture.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready)
    {
      update();
      rasterize();
    }

    _output.update();
  }

  SDL_Rect dest;
  //Texture* texture = SDL_CreateTextureFromSurface(renderer, machine.screen());
#ifdef _WIN32
  dest = { (480 - 384) / 2, (480 - 384) / 2, 384, 384 };
#else

if (_scaler == Scaler::UNSCALED)
  dest = { (320 - 128) / 2, (240 - 128) / 2, 128, 128 };
else if (_scaler == Scaler::SCALED_ASPECT_2x)
  dest = { (320 - 256) / 2, (240 - 256) / 2, 256, 256 };
else
  dest = { 0, 0, 320, 240 };
#endif

  manager->blitToScreen(_output, dest);
 
  if (_showFPS)
  {
    char buffer[16];
    sprintf(buffer, "%.0f/%c0", 1000.0f / manager->lastFrameTicks(), machine.code().require60fps() ? '6' : '3');
    manager->text(buffer, 10, 10);
  }

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

      Texture* texture = SDL_CreateTextureFromSurface(renderer, spritesheet);
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
          dest[j*16 + i] = colorTable.get(palette->get(r8::color_t(i)));
      }

      Texture* texture = SDL_CreateTextureFromSurface(renderer, palettes);
      SDL_Rect destr = { (1024 - 286) , 300, 256, 32 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(palettes);
    }


    {
      /*
      static SDL_Surface* tilemap = nullptr;

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

      Texture* texture = SDL_CreateTextureFromSurface(renderer, tilemap);
      SDL_Rect destr = { (1024 - 286) , 256, 256, 128 };
      SDL_RenderCopy(renderer, texture, nullptr, &destr);
      SDL_DestroyTexture(texture);
      SDL_FreeSurface(tilemap);*/
    }

  }
#endif
}

void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  switch (event.key.keysym.sym)
  {
  case SDLK_LEFT:
    _input.manageKey(0, 0, event.type == SDL_KEYDOWN);
    break;
  case SDLK_RIGHT:
    _input.manageKey(0, 1, event.type == SDL_KEYDOWN);
    break;
  case SDLK_UP:
    _input.manageKey(0, 2, event.type == SDL_KEYDOWN);
    break;
  case SDLK_DOWN:
    _input.manageKey(0, 3, event.type == SDL_KEYDOWN);
    break;

  case SDLK_z:
  case SDLK_LCTRL:
    _input.manageKey(0, 4, event.type == SDL_KEYDOWN);
    break;

  case SDLK_x:
  case SDLK_LALT:
    _input.manageKey(0, 5, event.type == SDL_KEYDOWN);
    break;

  case SDLK_a:
  case SDLK_SPACE:
    _input.manageKey(1, 4, event.type == SDL_KEYDOWN);
    break;

  case SDLK_s:
  case SDLK_LSHIFT:
    _input.manageKey(1, 5, event.type == SDL_KEYDOWN);
    break;

  case SDLK_m:
  {
    if (event.type == SDL_KEYDOWN)
    {
      bool s = machine.sound().isMusicEnabled();
      machine.sound().toggleMusic(!s);
      machine.sound().toggleSound(!s);
    }
    break;
  }

#if DESKTOP_MODE
  case SDLK_p:
    if (event.type == SDL_KEYDOWN)
      if (_paused)
        pause();
      else
        resume();
    break;
#else
  case SDLK_TAB:
    if (event.type == SDL_KEYDOWN)
    {
      if (_scaler < Scaler::LAST) _scaler = Scaler(_scaler + 1);
      else _scaler = Scaler::FIRST;
  }
    break;
#endif

  case SDLK_RETURN:
    manager->openMenu();
    break;

  case SDLK_ESCAPE:
    if (event.type == SDL_KEYDOWN)
      manager->exit();
    break;
}
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}

void GameView::pause()
{
  _paused = true;

#if SOUND_ENABLED
  sdlAudio.pause();
#endif
}

void GameView::resume()
{
  _paused = false;

#if SOUND_ENABLED
  sdlAudio.resume();
#endif
}

GameView::~GameView()
{
  _output.release();
  //TODO: the _init future is not destroyed
  sdlAudio.close();
}


uint32_t Platform::getTicks() { return SDL_GetTicks(); }