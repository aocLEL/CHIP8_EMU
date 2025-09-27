#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <graphics/display.h>
#include <error.h>
#include <chip8.h>

#define __private static

#define SDL_APP_NAME     "CHIP8_EMU"
#define SDL_APP_ID       ""
#define SDL_APP_VERSION  VERSION_STR 



__private sdl_s *init_sdl(sdl_s **sdl, const char *win_name, unsigned int res_x, unsigned int res_y) {
  SDL_SetAppMetadata(SDL_APP_NAME, SDL_APP_VERSION, SDL_APP_ID); //NOLINT
  if(!(*sdl = malloc(sizeof(sdl_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  // init subsystems
  SDL_InitFlags init_flags = SDL_INIT_VIDEO; 
  if(!SDL_InitSubSystem(init_flags)) {
    SDL_Log("Couldn't initialize SDL : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  // set up window and render
  SDL_WindowFlags win_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS; 
  if(!SDL_CreateWindowAndRenderer(win_name, res_x, res_y, win_flags, &(*sdl)->win, &(*sdl)->rnd)) {
    SDL_Log("Couldn't initialize SDL : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  return *sdl;
}

display_s *init_display(display_s **dp, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt) {
  if(!(*dp = malloc(sizeof(display_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  init_sdl(&(*dp)->hw, win_name, res_x, res_y);
  (*dp)->res_x  = res_x;
  (*dp)->res_y  = res_y;
  (*dp)->ref_rt = ref_rt;
  return *dp;
}



// we use 2D accelerated rendering, so pixels are created thorugh rendering functions, see pixelformat and textures


void display_free(display_s *dp) {
  SDL_DestroyRenderer(dp->hw->rnd);
  SDL_DestroyWindow(dp->hw->win);
  SDL_Quit();
  free(dp->hw);
  free(dp);
}
