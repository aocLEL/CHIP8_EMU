#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
  printf("SPF: 0x%x\n", *emu->mem->spf);
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
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->pc_r = jmp_addr;
  return 1;
}

unsigned int instr_jmpoff(emu_s *emu, chip_arg8 x_reg, chip_arg16 jmp_addr) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  instr_jmp(emu, jmp_addr + emu->cpu->gp_r[emu->chipmd ? x_reg : 0]);
  return 1;
}

unsigned int set_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = val;
  return 1;
}

unsigned int add_rrc(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  chip_arg16 res = emu->cpu->gp_r[x_reg] + emu->cpu->gp_r[y_reg];
  emu->cpu->gp_r[FLAG_REG] = res > 255 ? 1 : 0;
  emu->cpu->gp_r[x_reg] = (chip_arg8)res;
  return 1;
}

unsigned int instr_addvxix(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->i_r += emu->cpu->gp_r[x_reg];
  if(emu->cpu->i_r > 0xFFF)
    emu->cpu->gp_r[FLAG_REG] = 1;
  return 1;
}

unsigned int sub_rr(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[FLAG_REG] = 1;
  if(emu->cpu->gp_r[x_reg] < emu->cpu->gp_r[y_reg])
    emu->cpu->gp_r[FLAG_REG] = 0;
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] - emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int sub_rrrev(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[FLAG_REG] = 1;
  if(emu->cpu->gp_r[y_reg] < emu->cpu->gp_r[x_reg])
    emu->cpu->gp_r[FLAG_REG] = 0;
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg] - emu->cpu->gp_r[x_reg];
  return 1;
}



unsigned int reg_rshift(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  if(!emu->chipmd)
    emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg];
  emu->cpu->gp_r[FLAG_REG] = emu->cpu->gp_r[x_reg] & 0x1;
  emu->cpu->gp_r[x_reg] >>= 1;
  return 1;
}



unsigned int reg_lshift(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  if(!emu->chipmd)
    emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg];
  emu->cpu->gp_r[FLAG_REG] = emu->cpu->gp_r[x_reg] >> 7;
  emu->cpu->gp_r[x_reg] <<= 1;
  return 1;
}


unsigned int add_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = (emu->cpu->gp_r[x_reg] + val) % 0x100;
  return 1; 
}

unsigned int set_xy(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[y_reg];
  return 1;
}

// remember to protect from invalid memory accesses

unsigned int call_sub(emu_s *emu, chip_arg16 sub_addr) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  printf("Pushing 0x%x pc into stack!!\n", emu->cpu->pc_r);
  s_push(emu->mem, emu->cpu->pc_r);
  emu->cpu->pc_r = sub_addr;
  return 1;
}


unsigned int ret_sub(emu_s *emu) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  puts("ret sub");
  emu->cpu->pc_r = s_pop(emu->mem);
  printf("mem out %x\n", emu->cpu->pc_r);
  return 1;
}


unsigned int set_ixreg(emu_s *emu, chip_arg16 val) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->i_r = val;
  return 1;
}


unsigned int instr_setvxdt(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = emu->cpu->d_timer->value;
  return 1;
}


unsigned int instr_setdt(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  set_timer(emu->cpu->d_timer, emu->cpu->gp_r[x_reg]);
  return 1;
}


unsigned int instr_setst(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  set_timer(emu->cpu->s_timer, emu->cpu->gp_r[x_reg]);
  return 1;
}


// skip if
unsigned int skip_eq(emu_s *emu, chip_arg16 op1, chip_arg16 op2) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  if(op1 == op2)
    emu->cpu->pc_r += 2;
  return 1;
}

unsigned int skip_neq(emu_s *emu, chip_arg16 op1, chip_arg16 op2) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  if(op1 != op2)
    emu->cpu->pc_r += 2;
  return 1;
}

unsigned int instr_skkeypr(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  uint8_t key = emu->cpu->gp_r[x_reg];
  if(emu->dp->keypad[key].pressed)
    emu->cpu->pc_r += 2;
  return 1; 
}

unsigned int instr_skkeynpr(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  uint8_t key = emu->cpu->gp_r[x_reg];
  if(!emu->dp->keypad[key].pressed)
    emu->cpu->pc_r += 2;
  return 1; 
}


// logical
unsigned int bitwise_or(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] | emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int bitwise_and(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] & emu->cpu->gp_r[y_reg];
  return 1;
}

unsigned int bitwise_xor(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  emu->cpu->gp_r[x_reg] = emu->cpu->gp_r[x_reg] ^ emu->cpu->gp_r[y_reg];
  return 1;
}



// store in memory all registers from V0 to VX
unsigned int instr_stmem(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  st_mem(emu->mem, emu->cpu->i_r, emu->cpu->gp_r, x_reg + 1);
  if(emu->chipmd) 
    emu->cpu->i_r += x_reg + 1;
  return 1;
}


// load from memory all registers from V0 to VX
unsigned int instr_ldmem(emu_s *emu, chip_arg8 x_reg) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
  ld_mem(emu->mem, emu->cpu->i_r, emu->cpu->gp_r, x_reg + 1);
  if(emu->chipmd) 
    emu->cpu->i_r += x_reg + 1;
  return 1;
}

// get info about pixel at coordinates x and y
__private inline uint8_t get_pix_mode(uint8_t *pixmap, unsigned int x_c, unsigned int y_c) {
  return pixmap[PIXMAP_IDX(y_c, x_c)];
}


unsigned int draw_dp(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg, unsigned int n_pixel) {
  printf("SPF: 0x%x\n", *emu->mem->spf);
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



unsigned int instr_rand(emu_s *emu, chip_arg8 x_reg, chip_arg8 seed) {
  srand(time(NULL));
  emu->cpu->gp_r[x_reg] = seed & (rand() % (256));
  return 1;
}



unsigned int instr_getkey(emu_s *emu, chip_arg8 x_reg) {
  // check if some key is pressed in keypad, if yes put the first one in vx, if no simply decrement pc to repeat the instruction
  for(size_t i = 0; i < CHIP_KEYPAD_SIZE; i++) {
    if(emu->dp->keypad[i].pressed) {
      emu->cpu->gp_r[x_reg] = i;
      return 1;
    }
  }
  emu->cpu->pc_r -= 2;
  return 1;
}

unsigned int instr_fontchar(emu_s *emu, chip_arg8 x_reg) {
  emu->cpu->i_r = FONT_SADDR + ((chip_arg16)emu->cpu->gp_r[x_reg] * FONT_DIM);
  return 1;
}

unsigned int instr_bindec(emu_s *emu, chip_arg8 x_reg) {
  uint8_t num = emu->cpu->gp_r[x_reg];
  uint8_t decnum[3] = {0, 0, 0};
  size_t i = 3;
  while(num > 0) {
       decnum[--i] = num % 10;
       num /= 10;
  }
  if(i == 3) --i;
  st_mem(emu->mem, emu->cpu->i_r, decnum + i, 3 - i);
  return 1;
}
