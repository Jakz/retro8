#include "views/view_manager.h"
#include "views/main_view.h"

/*
* D-PAD Left - SDLK_LEFT
* D-PAD Right - SDLK_RIGHT
* D-PAD Up - SDLK_UP
* D-PAD Down - SDLK_DOWN
* Y button - SDLK_SPACE
* X button - SDLK_LSHIFT
* A button - SDLK_LCTRL
* B button - SDLK_LALT
* START button - SDLK_RETURN
* SELECT button - SDLK_ESC
* L shoulder - SDLK_TAB
* R shoulder - SDLK_BACKSPACE
* Power slider in up position - SDLK_POWER (not encouraged to map in game, as it's used by the pwswd daemon)
* Power slider in down position - SDLK_PAUSE
*/

#if TEST_MODE
extern int testMain(int argc, char* argv[]);
#endif

int main(int argc, char* argv[])
{
#if TEST_MODE
  return testMain(argc, argv);
#endif

  ui::ViewManager ui;

  if (!ui.init())
    return -1;
  
  if (!ui.loadData())
  {
    printf("Error while loading and initializing data.\n");
    ui.deinit();
    return -1;
  }

  if (argc == 2)
    ui.gameView()->loadCartridge(argv[1]);
  
  ui.loop();
  ui.deinit();

  return 0;
}
