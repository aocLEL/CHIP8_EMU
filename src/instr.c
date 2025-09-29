#include <stdio.h>
#include <stdlib.h>
#include <instr.h>
#include <SDL3/SDL.h>
#include <error.h>

#define FLAG_REG  0xF
#define PIX_ON    1  
#define PIX_OFF   0


#define __private static
// screen refreshes (Present) are made by main emu loop



// return a symbolic positive value on success

unsigned int instr_clear_screen(emu_s *emu) {
  // sets all pixels to black(0)
  memset(emu->dp->pixmap, 0, sizeof(uint8_t) * CHIP_DPH * CHIP_DPW);
  if(!SDL_SetRenderDrawColor(emu->dp->hw->rnd, OFF_COLOR_R, OFF_COLOR_G, OFF_COLOR_B, SDL_ALPHA_OPAQUE)) {
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

unsigned int add_rrc(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  chip_arg16 res = emu->cpu->gp_r[x_reg] + emu->cpu->gp_r[y_reg];
  emu->cpu->gp_r[FLAG_REG] = res > 255 ? 1 : 0;
  emu->cpu->gp_r[x_reg] = (chip_arg8)res;
  return 1;
}

unsigned int sub_rr(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[FLAG_REG] = 1;
  if(emu->cpu->gp_r[x_reg] < emu->cpu->gp_r[y_reg])
    emu->cpu->gp_r[FLAG_REG] = 0;
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] - emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int sub_rrrev(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[FLAG_REG] = 1;
  if(emu->cpu->gp_r[y_reg] < emu->cpu->gp_r[x_reg])
    emu->cpu->gp_r[FLAG_REG] = 0;
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg] - emu->cpu->gp_r[x_reg];
  return 1;
}


unsigned int add_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val) {
  emu->cpu->gp_r[x_reg] = (emu->cpu->gp_r[x_reg] + val) % 0x100;
  return 1; 
}

unsigned int set_xy(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg];
  return 1;
}

// remember to protect from invalid memory accesses

unsigned int call_sub(emu_s *emu, chip_arg16 sub_addr) {
  s_push(emu->mem, emu->cpu->pc_r);
  emu->cpu->pc_r = sub_addr;
  return 1;
}


unsigned int ret_sub(emu_s *emu) {
  emu->cpu->pc_r = s_pop(emu->mem);
  return 1;
}


unsigned int set_ixreg(emu_s *emu, chip_arg16 val) {
  emu->cpu->i_r = val;
  return 1;
}


// skip if
unsigned int skip_eq(emu_s *emu, chip_arg16 op1, chip_arg16 op2) {
  if(op1 == op2)
    emu->cpu->pc_r += 2;
  return 1;
}

unsigned int skip_neq(emu_s *emu, chip_arg16 op1, chip_arg16 op2) {
  if(op1 != op2)
    emu->cpu->pc_r += 2;
  return 1;
}


// logical
unsigned int bitwise_or(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] | emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int bitwise_and(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] & emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int bitwise_xor(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] ^ emu->cpu->gp_r[y_reg];
  return 1;
}

// get info about pixel at coordinates x and y
__private inline uint8_t get_pix_mode(uint8_t *pixmap, unsigned int x_c, unsigned int y_c) {
  return pixmap[PIXMAP_IDX(y_c, x_c)];
}


unsigned int draw_dp(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg, unsigned int n_pixel) {
  uint8_t      *pixmap = emu->dp->pixmap;
  unsigned int x_c     = emu->cpu->gp_r[x_reg] % CHIP_DPW;
  unsigned int y_c     = emu->cpu->gp_r[y_reg] % CHIP_DPH;
  emu->cpu->gp_r[FLAG_REG] = 0;
  for(size_t i = 0; i < n_pixel; i++) {
    unsigned int  tmpx_c  = x_c;
    uint8_t       px_byte = *(emu->mem->mem_base + emu->cpu->i_r + i);
    for(size_t k = 0; k < 8; k++) {
      uint8_t mbit = px_byte >> 7;
      px_byte <<= 1;
      uint8_t pixmode = get_pix_mode(pixmap, tmpx_c, y_c);
      if(mbit) {
        if(pixmode)
          emu->cpu->gp_r[FLAG_REG] = 1;
        pixmap[PIXMAP_IDX(y_c, tmpx_c)] = !pixmode;
      }
      tmpx_c++;
    }
    y_c++;
  }
  return 1;
}
