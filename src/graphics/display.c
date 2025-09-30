#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <graphics/display.h>
#include <error.h>
#include <chip8.h>

#define __private static

#define SDL_APP_NAME     "CHIP8_EMU"
#define SDL_APP_ID       ""
#define SDL_APP_VERSION  VERSION_STR 


// keymap structure, maps from key scancode to chip8 key structure in keypad array
int8_t        keymap[INT8_MAX + 1];
// default keypad
const uint8_t def_keypad[CHIP_KEYPAD_SIZE] = {'1', '2', '3', '4', 'q', 'w', 'e', 'r', 'a', 's', 'd', 'f', 'z', 'x', 'c', 'v'};


__private sdl_s *init_sdl(sdl_s **sdl, const char *win_name, unsigned int res_x, unsigned int res_y) {
  /*
  if(!SDL_SetAppMetadata(SDL_APP_NAME, SDL_APP_VERSION, SDL_APP_ID)) {
    SDL_Log("Couldn't initialize SDL metadatas: %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  */
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
  if(!SDL_SetWindowPosition((*sdl)->win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED)) {
    SDL_Log("Couldn't center the window : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  return *sdl;
}


__private void set_keypad(chip8_key_s *kp, optval_u *rkp) {
  memset(keymap, -1, INT8_MAX + 1);
  if(rkp) {
   // set to user defined keys 
   for(size_t i = 0; i < CHIP_KEYPAD_SIZE; i++) {
     uint8_t key = rkp[i].str[0];
     if(!isalnum(key)) {
       // this have to be replaced by emu_die !!!
        fprintf(stderr, "Keypad keys have to be alphanumeric (%c is not)!!!\n", key);
        exit(EXIT_FAILURE);
     }
     kp[i].keycode = key;
     kp[i].pressed = 0;
     keymap[key] = i;
   }
  }
  else {
    for(size_t i = 0; i < CHIP_KEYPAD_SIZE; i++) {
      kp[i].keycode = def_keypad[i];
      kp[i].pressed = 0;
      keymap[kp[i].keycode] = i;
    }
  }
}

inline int8_t get_keymap(int8_t idx) {
  return keymap[idx];
}

display_s *init_display(display_s **dp, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt, optval_u *raw_keypad) {
  if(!(*dp = malloc(sizeof(display_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  init_sdl(&(*dp)->hw, win_name, res_x, res_y);
  (*dp)->res_x  = res_x;
  (*dp)->res_y  = res_y;
  (*dp)->ref_rt = ref_rt;
  // initialize pixmap
  unsigned int pixmap_dim = sizeof(uint8_t) * CHIP_DPW * CHIP_DPH;
  if(!((*dp)->pixmap = malloc(pixmap_dim)))
    die("Memory allocation failed: %s", strerror(errno));
  memset((*dp)->pixmap, 0,  pixmap_dim);
  // setting keypad
  if(!((*dp)->keypad = malloc(sizeof(chip8_key_s) * CHIP_KEYPAD_SIZE))) 
    die("Memory allocation failed: %s", strerror(errno));
  set_keypad((*dp)->keypad, raw_keypad);
  return *dp;
}



// we use 2D accelerated rendering, so pixels are created thorugh rendering functions, see pixelformat and textures


void display_free(display_s *dp) {
  SDL_DestroyRenderer(dp->hw->rnd);
  SDL_DestroyWindow(dp->hw->win);
  SDL_Quit();
  free(dp->pixmap);
  free(dp->keypad);
  free(dp->hw);
  free(dp);
}
