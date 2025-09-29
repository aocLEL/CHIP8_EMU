#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SDL3/SDL.h>
#include <opt.h>


#define CHIP_DPW            64
#define CHIP_DPH            32
#define CHIP_KEYPAD_SIZE    16
#define DEFAULT_REAL_DPW    1280
#define DEFAULT_REAL_DPH    720
#define DEFAULT_RFR         60
#define DEFAULT_KEYPAD      NULL


#define PIXMAP_IDX(r, c) ((r) * CHIP_DPW + (c))

typedef struct {
  SDL_Window    *win;
  SDL_Renderer  *rnd;
} sdl_s;



typedef struct {
  uint8_t keycode;
  uint8_t pressed;
} chip8_key_s;


typedef struct {
  unsigned int res_x;
  unsigned int res_y;
  unsigned int ref_rt;
  sdl_s        *hw;
  uint8_t      *pixmap;
  chip8_key_s  *keypad;
} display_s;


display_s *init_display(display_s **dp, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt, optval_u *raw_keypad);
int8_t    get_keymap(int8_t idx);
void      display_free(display_s *dp);

#endif
