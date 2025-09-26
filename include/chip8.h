#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdint.h>
#include <error.h>

#define PROG_MEM_SIZE 0x1000
#define PROG_ENTRY    0x200
#define PROG_UBOUND   PROG_ENTRY + 0xFFF
#define FONT_SADDR    0x50
#define FONT_EADDR    0x9f
#define NULL_ADDR     0x0

#define STACK_SIZE    0x100
#define STACK_LBOUND  PROG_MEM_SIZE 
#define STACK_UBOUND  STACK_LBOUND + STACK_SIZE  

#define MEM_SIZE      PROG_MEM_SIZE + STACK_SIZE

// timer frequency in hz
#define TIMER_FREQ    0x3c
// clock default freq, 700hz
#define CLOCK_DEFAULT 0x2bc

#define GPREGS_NUM    0x10

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



uint8_t fontset[] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// probably has to be implemented in SDL 
uint8_t keypad[] = {

};





// EMU components
typedef struct {
  int          value;
  unsigned int sound; // bool, is a sound timer
} timer_s;



timer_s *init_timer(int init_val, unsigned int sound);
int     dec_timer(timer_s *tm);
timer_s *set_timer(timer_s *tm, int val);




typedef struct {
  timer_s     *d_timer;
  timer_s     *s_timer;
  uint16_t    clock_speed; // configurable through command line arg, default otherwise
  // regs
  cpu_reg16   pc_r; // program counter
  cpu_reg16   i_r; // index register
  cpu_reg8    gp_r[GPREGS_NUM]; // gp registers V0-VF
} cpu_s;


typedef struct {
  char      *mem_base;
  uint16_t  sp_off;
} mem_s;

typedef struct {
  cpu_s     *cpu;
  mem_s     *mem; // 4kib
  display_s *dp; // stl display handler
} emu_s;


// emu funcs
const emu_s *emu_ctor(emu_s **emu, uint16_t clock_s); 
mem_s       *s_push(mem_s *mem, uint16_t val);
uint16_t    s_pop(mem_s *mem);

const emu_s *load_rom(emu_s *emu, const char *prog_name);
void        free_emu(emu_s *emu);

#endif
