#ifndef __INSTR_H__
#define __INSTR_H__

#include <stdint.h>
#include "chip8.h"

#define INSTR_CLEAR_SCREEN        0x00E0
#define INSTR_JUMP                0x1000
#define INSTR_SETVX               0x6000
#define INSTR_ADDVX               0x7000
#define INSTR_RETURN_SUB          0x00EE
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
unsigned int instr_jmp(emu_s *emu, chip_arg16 jmp_addr);
unsigned int instr_jmpoff(emu_s *emu, chip_arg8 x_reg, chip_arg16 jmp_addr);
unsigned int instr_add_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val);
unsigned int instr_add_rrc(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_addvxix(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_sub_rr(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_sub_rrrev(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_reg_rshift(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_reg_lshift(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_set_ixreg(emu_s *emu, chip_arg16 val);
unsigned int instr_setvxdt(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_setdt(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_setst(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_set_vreg(emu_s *emu, chip_arg8 x_reg, chip_arg8 val);
unsigned int instr_set_xy(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);

//logical
unsigned int instr_bitwise_or(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_bitwise_and(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);
unsigned int instr_bitwise_xor(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg);

unsigned int instr_call_sub(emu_s *emu, chip_arg16 sub_addr);
unsigned int instr_ret_sub(emu_s *emu);

unsigned int instr_stmem(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_ldmem(emu_s *emu, chip_arg8 x_reg);

// skip if
unsigned int instr_skip_eq(emu_s *emu, chip_arg16 op1, chip_arg16 op2);
unsigned int instr_skip_neq(emu_s *emu, chip_arg16 op1, chip_arg16 op2);
unsigned int instr_skkeypr(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_skkeynpr(emu_s *emu, chip_arg8 x_reg);

unsigned int instr_rand(emu_s *emu, chip_arg8 x_reg, chip_arg8 seed);
unsigned int instr_draw_dp(emu_s *emu, chip_arg8 x_reg, chip_arg8 y_reg, unsigned int n_pixel);
unsigned int instr_clear_screen(emu_s *emu);
unsigned int instr_getkey(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_fontchar(emu_s *emu, chip_arg8 x_reg);
unsigned int instr_bindec(emu_s *emu, chip_arg8 x_reg);



#endif
