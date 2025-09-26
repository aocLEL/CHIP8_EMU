#include <chip8.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define __private static


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
  assert(FONT_SADDR + sizeof(fontset) == FONT_EADDR);
  memcpy(emu->mem->mem_base + FONT_SADDR, fontset, sizeof(fontset));
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

const emu_s *emu_ctor(emu_s **emu, uint16_t clock_s) {
  // creating emu object
  if(!(*emu = malloc(sizeof(emu_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  init_cpu(*emu, clock_s);
  init_mem(*emu);
  // remember to optimize for page alignment and other
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
  return emu;
}








void free_emu(emu_s *emu) {
  free(emu->mem->mem_base);
  free(emu->mem);
  free(emu->cpu->s_timer);
  free(emu->cpu->d_timer);
  free(emu->cpu);
  free(emu);
}
