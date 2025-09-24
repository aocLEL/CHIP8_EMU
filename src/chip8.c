#include <chip8.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <errno.h>



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


const emu_s *emu_ctor(emu_s *emu, const char *prog_name, uint16_t clock_s) {
  // init cpu
  cpu_s *cpu;
  if(!(cpu = malloc(sizeof(cpu_s))))
    die("Memory allocation failed: %s", strerror(errno));
  cpu->clock_speed = clock_s;
  cpu->i_r = NULL_ADDR;
  cpu->s_timer = init_timer(TIMER_FREQ, 0);
  cpu->d_timer = init_timer(TIMER_FREQ, 1);
  // creating emu object
  if(!(emu = malloc(sizeof(emu_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  emu->cpu = cpu;
  // setting up program virtual memory space
  // TODO:
  // - write functions to push/pop from the stack, handling stack overflow/underflow
  // - alloc memory
  // - parse prog_name rom and load it into memory
  // remember to optimize for page alignment and other
  


}



