#ifndef __INSTR_H__
#define __INSTR_H__

#include <stdint.h>
#include "chip8.h"

#define INSTR_CLEAR_SCREEN        0x00E0
#define INSTR_JUMP                0x1000
#define INSTR_SETVX               0x6000
#define INSTR_ADDVX               0x7000
#define INSTR_SETIX               0xA000
#define INSTR_DRAW                0xD000

#define ON_COLOR_R    255
#define ON_COLOR_G    255
#define ON_COLOR_B    255

#define OFF_COLOR_R   0
#define OFF_COLOR_G   0
#define OFF_COLOR_B   0


#define chip_arg16 uint16_t
#define chip_arg8  uint8_t

// instructions
unsigned int instr_clear_screen(emu_s *emu);
unsigned int instr_jmp(emu_s *emu, chip_arg16 jmp_addr);
unsigned int set_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val);
unsigned int add_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val);
unsigned int set_ixreg(emu_s *emu, chip_arg16 val);
unsigned int draw_dp(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg, unsigned int n_pixel);

#endif
