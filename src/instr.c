#include <stdio.h>
#include <stdlib.h>
#include <instr.h>
#include <SDL3/SDL.h>
#include <error.h>

#define FLAG_REG  0xF
#define PIX_ON    1  
#define PIX_OFF   0

#define ON_COLOR_R    255
#define ON_COLOR_G    255
#define ON_COLOR_B    255

#define OFF_COLOR_R   0
#define OFF_COLOR_G   0
#define OFF_COLOR_B   0


#define __private static
// screen refreshes (Present) are made by main emu loop

// return a symbolic positive value on success

unsigned int instr_clear_screen(emu_s *emu) {
  // sets all pixels to black(0)
  if(!SDL_SetRenderDrawColor(emu->dp->hw->rnd, 0, 0, 0, 255)) {
    SDL_Log("Couldn't draw color on screen : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  if(!SDL_RenderClear(emu->dp->hw->rnd)) {
    SDL_Log("Couldn't clear the screen : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  return 1;
}


unsigned int instr_jmp(emu_s *emu, chip_arg16 jmp_addr) {
  emu->cpu->pc_r = jmp_addr;
  return 1;
}


unsigned int set_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val) {
  emu->cpu->gp_r[x_reg] = val;
  return 1;
}


unsigned int add_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val) {
  emu->cpu->gp_r[x_reg] = (emu->cpu->gp_r[x_reg] + val) % 0x100;
  return 1; 
}


unsigned int set_ixreg(emu_s *emu, chip_arg16 val) {
  emu->cpu->i_r = val;
  return 1;
}


// draw a pixel on the renderer, pixel is  a chip8 pixel scaled for the current resolution, pixel is put on chosen color(on or off)
// args are emu x and y start coords and scaled pixel dims
__private void draw_pixel(emu_s *emu, unsigned int x_c, unsigned int y_c, unsigned int xpix_dim, unsigned int ypix_dim, uint8_t mode) {
  uint8_t color[3];
  if(mode) {
    // white
    color[0] = ON_COLOR_R;
    color[1] = ON_COLOR_G;
    color[2] = ON_COLOR_B;
  } else {
    color[0] = OFF_COLOR_R;
    color[1] = OFF_COLOR_G;
    color[2] = OFF_COLOR_B;
  }
  if(!SDL_SetRenderDrawColor(emu->dp->hw->rnd, color[0], color[1], color[2], SDL_ALPHA_OPAQUE)) {
    SDL_Log("Couldn't set pixel color : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  // see what happen if reach end of display, if truncates is ok
  if(!SDL_RenderLine(emu->dp->hw->rnd, x_c, y_c, x_c + xpix_dim, y_c + ypix_dim)) {
    SDL_Log("Couldn't draw pixel : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
}

// get info about pixel at coordinates x and y
__private uint8_t get_pix_mode(emu_s *emu, unsigned int x_c, unsigned int y_c) {
  SDL_Surface *surf = SDL_RenderReadPixels(emu->dp->hw->rnd, &(SDL_Rect){x_c, y_c, 1, 1});
  if(!surf) {
    SDL_Log("Couldn't read pixels : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  // reading pixel colors
  uint8_t r, g, b;
  // speriamo che il formato di default sia giusto
  SDL_GetRGB(((uint32_t*)surf->pixels)[0], SDL_GetPixelFormatDetails(surf->format), NULL, &r, &g, &b);
  uint8_t mode = 0;
  // if on color
  if(r == ON_COLOR_R && g == ON_COLOR_G && b == ON_COLOR_B) 
    mode = 1;
  SDL_DestroySurface(surf);
  return mode;
}


unsigned int draw_dp(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg, unsigned int n_pixel) {
  unsigned int x_c = emu->cpu->gp_r[x_reg];
  unsigned int y_c = emu->cpu->gp_r[y_reg];
  // map chip8 coords to real ones based on screen size
  x_c = ((x_c * emu->dp->res_x) / CHIP_DPW) % emu->dp->res_x;
  y_c = ((y_c * emu->dp->res_y) / CHIP_DPH) % emu->dp->res_y;
  // size of each scaled pixel (x and y)
  unsigned int sc_xpix_dim = ((8 * emu->dp->res_x) / CHIP_DPW) % emu->dp->res_x;
  unsigned int sc_ypix_dim = (emu->dp->res_y / CHIP_DPH);
  for(size_t i = 0; i < n_pixel; i++) {
    // get n-byte of the sprite from IX
    uint8_t px_byte = *(emu->mem->mem_base + emu->cpu->i_r + i);
    // for each pixel in the byte
    for(size_t k = 0; k < 8; k++) {
      // get nth bit from left
      uint8_t mbit = px_byte >> 7;
      px_byte <<= 1;
      // flip mbit in the display
      // search into that scaled pixel line at scaled pixel k in the row, see if it's on or off
      uint8_t pix_mode = get_pix_mode(emu, x_c + sc_xpix_dim * k, y_c); 
      // if mbit, flip display bit, if it was on, set flag reg
      if(mbit) {
        if(pix_mode)
          emu->cpu->gp_r[FLAG_REG] = 1;
        draw_pixel(emu, x_c + sc_xpix_dim * k, y_c + sc_ypix_dim * i, sc_xpix_dim, sc_ypix_dim, !pix_mode);
      }
    }
  }
  return 1;
}
