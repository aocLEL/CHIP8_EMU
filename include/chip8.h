#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdint.h>
#include <error.h>
#include <opt.h>
#include "graphics/display.h"

#define PROG_MEM_SIZE 0x1000
#define PROG_ENTRY    0x200
#define PROG_UBOUND   (PROG_ENTRY + 0xFFF)
#define FONT_SADDR    0x50
#define FONT_EADDR    0x9f
#define NULL_ADDR     0x0

#define STACK_SIZE    0x100
#define STACK_LBOUND  (PROG_UBOUND + 1) 
#define STACK_UBOUND  (STACK_LBOUND + STACK_SIZE)  

#define MEM_SIZE      (PROG_MEM_SIZE + STACK_SIZE)

// timer frequency in hz
#define TIMER_FREQ    0x3c
// NS in S
#define NS_S          1000000000LL
// clock default freq, 700hz
#define CLOCK_DEFAULT 0x2bc
// default shift behavior (early)
#define DEFAULT_SHIFT 0x0

#define GPREGS_NUM    0x10

#define FONT_DIM      0x5

// istruction parsing macros
#define INSTR_OPCODE(x) (((x) >> 12) & 0xf)
#define INSTR_X(x)      (((x) >> 8) & 0xf)
#define INSTR_Y(x)      (((x) >> 4) & 0xf)
#define INSTR_N(x)      ((x) & 0xf)
#define INSTR_NN(x)     ((x) & 0xff)
#define INSTR_NNN(x)    ((x) & 0xfff)

// type aliases
typedef uint16_t cpu_reg16;
typedef uint16_t instr_t;
typedef uint8_t  cpu_reg8;
typedef uint8_t* mem_p;



// EMU components
typedef struct {
  int          value;
  unsigned int sound; // bool, is a sound timer
} timer_s;



timer_s *init_timer(int init_val, unsigned int sound);
int     dec_timer(timer_s *tm);
timer_s *set_timer(timer_s *tm, int val);


typedef struct {
  uint16_t clock_speed; // clock freq
  uint64_t cycle_duration_ns; // duration of each cycle based on freq
  uint64_t curr; // curr time slice
} clock_s;


typedef struct {
  timer_s     *d_timer;
  timer_s     *s_timer;
  clock_s     *clock;
  // regs
  cpu_reg16   pc_r; // program counter
  cpu_reg16   i_r; // index register
  cpu_reg8    gp_r[GPREGS_NUM]; // gp registers V0-VF
} cpu_s;


typedef struct {
  mem_p     mem_base;
  uint16_t  *sp;
  uint16_t  *spf;
} mem_s;


typedef struct {
  cpu_s     *cpu;
  mem_s     *mem; // 4kib
  display_s *dp; // stl display handler
  uint8_t   chipmd;
} emu_s;


// emu funcs
const emu_s *emu_ctor(emu_s **emu, uint16_t clock_s, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt, uint8_t chipmd, optval_u *raw_keypad); 
void        emu_loop(emu_s *emu);
mem_s       *s_push(emu_s *emu, uint16_t val);
uint16_t    s_pop(emu_s *emu);
uint8_t     *st_mem(emu_s *emu, uint16_t addr, const uint8_t *src, uint8_t size);
uint8_t     *ld_mem(emu_s *emu, uint16_t addr, uint8_t *dest, uint8_t size);

const emu_s *load_rom(emu_s *emu, const char *prog_name);
void        free_emu(emu_s *emu);

#endif
