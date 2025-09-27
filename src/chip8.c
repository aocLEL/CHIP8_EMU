#include <chip8.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <instr.h>

#define __private static




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

uint8_t keypad[] = {

};

timer_s *init_timer(int init_val, unsigned int sound) {
  timer_s *tm;
  if(!(tm = malloc(sizeof(timer_s))))
    die("Memory allocation failed: %s", strerror(errno));
  tm->value = init_val;
  tm->sound = sound;
  return tm;
}

timer_s *set_timer(timer_s *tm, int val) {
  tm->value = val;
  return tm;
}

inline int dec_timer(timer_s *tm) {
  return --tm->value ? tm->value : (tm->value = 60); 
}


mem_s *s_push(mem_s *mem, uint16_t val) {
  char *sp = mem->mem_base + mem->sp_off;
  if(mem->sp_off >= STACK_UBOUND)
    die("*** Stack Overflow *** ABORT!!!");
  memcpy(sp, &val, 2);
  mem->sp_off += 2;
  return mem;
}

uint16_t s_pop(mem_s *mem) {
  char *sp = mem->mem_base + mem->sp_off;
  if(mem->sp_off <= STACK_LBOUND) 
    die("*** Stack underflow *** ABORT!!!");
  uint16_t val = *((uint16_t *)sp);
  mem->sp_off -= 2;
  return val;
}


__private mem_s *init_mem(emu_s *emu) {
  mem_s *mem;
  if(!(mem = malloc(sizeof(mem_s))))
    die("Memory allocation failed: %s", strerror(errno));
  // allocating virtual memory space and setting sp
  if(!(mem->mem_base = malloc(sizeof(char) * MEM_SIZE)))
    die("Memory allocation failed: %s", strerror(errno));
  mem->sp_off = STACK_LBOUND;
  // loading fontset
  assert(FONT_SADDR + sizeof(fontset) - 1 == FONT_EADDR);
  memcpy(mem->mem_base + FONT_SADDR, fontset, sizeof(fontset));
  emu->mem = mem;
  return mem; 
}

__private cpu_s *init_cpu(emu_s *emu, uint16_t clock_s) {
  if(!(emu->cpu = malloc(sizeof(cpu_s))))
    die("Memory allocation failed: %s", strerror(errno));
  emu->cpu->clock_speed = clock_s;
  emu->cpu->i_r = NULL_ADDR;
  emu->cpu->s_timer = init_timer(TIMER_FREQ, 0);
  emu->cpu->d_timer = init_timer(TIMER_FREQ, 1);
  return emu->cpu;
}

const emu_s *emu_ctor(emu_s **emu, uint16_t clock_s, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt) {
  // creating emu object
  if(!(*emu = malloc(sizeof(emu_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  init_cpu(*emu, clock_s);
  init_mem(*emu);
  // remember to optimize for page alignment and other
  // init display
  init_display(&(*emu)->dp, win_name, res_x, res_y, ref_rt);
  return *emu;
}


const emu_s *load_rom(emu_s *emu, const char *prog_name) {
  // read file
  int fd;
  if((fd = open(prog_name, O_RDONLY)) == -1) {
    fprintf(stderr, "Can't open specified ROM: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  char *prog_mem = emu->mem->mem_base + PROG_ENTRY;
  if((read(fd, prog_mem, PROG_MEM_SIZE - PROG_ENTRY)) == -1) {
    fprintf(stderr, "Can't load rom into memory: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if(close(fd) == -1) {
    fprintf(stderr, "Error while closing file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  // set pc to first instruction
  emu->cpu->pc_r = PROG_ENTRY;
  return emu;
}



// fetch instruction at PC and increment it
__private instr_t fetch_instr(emu_s *emu) {
  instr_t n_instr = *((instr_t*)(emu->mem->mem_base + emu->cpu->pc_r));
  printf("DBG INSTR: %X\n", n_instr);
  emu->cpu->pc_r += 2;
  return n_instr;
}

// instruction handling functions
__private unsigned int exec_instr(emu_s *emu) {
 // fetch
 instr_t instr = fetch_instr(emu);
 cpu_reg8 opcode = INSTR_OPCODE(instr);
 switch(opcode) {
   case 0x0:
      // clear screen
      instr_clear_screen(emu);
      break;
   case 0x1:
      // jump
      instr_jmp(emu,  INSTR_NNN(instr));
      break;
   case 0x6:
      // set register vx at NN value
      set_vreg(emu, INSTR_X(instr), INSTR_NN(instr));
      break;
   case 0x7:
      // add value NN to register vx, needs to modify flag register
      add_vreg(emu, INSTR_X(instr), INSTR_NN(instr));
      break;
   case 0xA:
      // set ix to NNN
      set_ixreg(emu, INSTR_NNN(instr));
      break;
   case 0xD:
      // draw on screen
      draw_dp(emu, INSTR_X(instr), INSTR_Y(instr), INSTR_N(instr));
      break;
   default:
      die("*** EMU ERROR: UNRECOGNIZED INSTRUCTION %hu ***", instr);
 }
 // decode
 // execute
}

// remove unused
void emu_loop(emu_s *emu __attribute__((unused))) {
  SDL_Event e;
  bool quit = false;
  SDL_SetRenderDrawColor(emu->dp->hw->rnd, 0, 0, 0, 255);
  SDL_RenderClear(emu->dp->hw->rnd);
  SDL_RenderPresent(emu->dp->hw->rnd);
  while (!quit){
      while (SDL_PollEvent(&e)){
          if (e.type == SDL_EVENT_QUIT){
              quit = true;
          }
          if (e.type == SDL_EVENT_KEY_DOWN){
              quit = true;
          }
          if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
              quit = true;
          }
      }
  }
}





void free_emu(emu_s *emu) {
  free(emu->mem->mem_base);
  free(emu->mem);
  free(emu->cpu->s_timer);
  free(emu->cpu->d_timer);
  free(emu->cpu);
  free(emu);
}
