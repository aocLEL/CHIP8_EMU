#include <chip8.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <instr.h>
#include <arpa/inet.h>
#include <SDL3/SDL.h>

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
  if(tm->value > 0) --tm->value;
  return tm->value;
}


mem_s *s_push(mem_s *mem, uint16_t val) {
  mem_p sp = mem->mem_base + mem->sp_off;
  if(mem->sp_off >= STACK_UBOUND)
    die("*** Stack Overflow *** ABORT!!!");
  memcpy(sp, &val, 2);
  mem->sp_off += 2;
  return mem;
}

uint16_t s_pop(mem_s *mem) {
  mem->sp_off -= 2;
  mem_p sp = mem->mem_base + mem->sp_off;
  if(mem->sp_off < STACK_LBOUND) 
    die("*** Stack underflow *** ABORT!!!");
  uint16_t val = *((uint16_t *)sp);
  return val;
}


__private mem_s *init_mem(emu_s *emu) {
  mem_s *mem;
  if(!(mem = malloc(sizeof(mem_s))))
    die("Memory allocation failed: %s", strerror(errno));
  // allocating virtual memory space and setting sp
  if(!(mem->mem_base = malloc(sizeof(unsigned char) * MEM_SIZE)))
    die("Memory allocation failed: %s", strerror(errno));
  memset(mem->mem_base, 0, sizeof(unsigned char) * MEM_SIZE);
  mem->sp_off = STACK_LBOUND;
  // loading fontset
  assert(FONT_SADDR + sizeof(fontset) - 1 == FONT_EADDR);
  memcpy(mem->mem_base + FONT_SADDR, fontset, sizeof(fontset));
  emu->mem = mem;
  return mem; 
}


uint8_t *st_mem(mem_s *mem, uint16_t addr, const uint8_t *src, uint8_t size) {
  if(size + addr >= MEM_SIZE)
    die("*** Segmentation Fault ***");
  memcpy(mem->mem_base + addr, src, size);
  return (uint8_t*)mem->mem_base + addr;
}



uint8_t *ld_mem(mem_s *mem, uint16_t addr, uint8_t *dest, uint8_t size) {
  if(size + addr >= MEM_SIZE)
    die("*** Segmentation Fault ***");
  memcpy(dest, mem->mem_base + addr, size);
  return dest;
}


// clock funcs
__private void init_clock(clock_s **clk, uint16_t clock_sp) {
  if(!(*clk = malloc(sizeof(clock_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  (*clk)->clock_speed = clock_sp;
  (*clk)->curr = 0;
  (*clk)->cycle_duration_ns = NS_S / (*clk)->clock_speed;
}


__private uint64_t get_time_ns() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * NS_S + ts.tv_nsec;
}

__private inline void clock_start(clock_s *clk) {
  clk->curr = get_time_ns();
}

// clock sync, sleep for remaining time and reset time slice for next instruction
__private void clock_sync(clock_s *clk) {
  // sleep for the remaining time
  uint64_t elapsed = get_time_ns() - clk->curr;
  long sleep_time  = clk->cycle_duration_ns - elapsed;
  if (sleep_time > 0) {
    struct timespec ts;
    ts.tv_sec = sleep_time / NS_S;
    ts.tv_nsec = sleep_time % NS_S;
    nanosleep(&ts, NULL);
  }
  // start another cycle
  clock_start(clk);
}

__private inline unsigned int clock_exp(clock_s *clk) {
  uint64_t elapsed = get_time_ns() - clk->curr;
  return elapsed >= clk->cycle_duration_ns;
}


__private cpu_s *init_cpu(emu_s *emu, uint16_t clock_s) {
  if(!(emu->cpu = malloc(sizeof(cpu_s))))
    die("Memory allocation failed: %s", strerror(errno));
  init_clock(&emu->cpu->clock, clock_s);
  emu->cpu->i_r = NULL_ADDR;
  emu->cpu->s_timer = init_timer(TIMER_FREQ, 0);
  emu->cpu->d_timer = init_timer(TIMER_FREQ, 1);
  return emu->cpu;
}

const emu_s *emu_ctor(emu_s **emu, uint16_t clock_s, const char *win_name, unsigned int res_x, unsigned int res_y, unsigned int ref_rt, uint8_t chipmd, optval_u *raw_keypad) {
  // creating emu object
  if(!(*emu = malloc(sizeof(emu_s)))) 
    die("Memory allocation failed: %s", strerror(errno));
  init_cpu(*emu, clock_s);
  init_mem(*emu);
  // remember to optimize for page alignment and other
  // init display
  init_display(&(*emu)->dp, win_name, res_x, res_y, ref_rt, raw_keypad);
  (*emu)->chipmd = chipmd;
  return *emu;
}


const emu_s *load_rom(emu_s *emu, const char *prog_name) {
  // read file
  int fd;
  if((fd = open(prog_name, O_RDONLY)) == -1) {
    fprintf(stderr, "Can't open specified ROM: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  mem_p prog_mem = emu->mem->mem_base + PROG_ENTRY;
  if((read(fd, prog_mem, PROG_MEM_SIZE)) == -1) {
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
  instr_t *pc = (instr_t*)(emu->mem->mem_base + emu->cpu->pc_r);
  instr_t n_instr = htons(*pc);
  emu->cpu->pc_r += 2;
  return n_instr;
}

// instruction handling functions
__private unsigned int exec_instr(emu_s *emu) {
 // fetch
 instr_t instr = fetch_instr(emu);
 printf("Executing instr %X at addr 0x%x\n", instr, emu->cpu->pc_r - 2);
 cpu_reg8 opcode = INSTR_OPCODE(instr);
 switch(opcode) {
   case 0x0:
      // clear screen
      if(instr == INSTR_CLEAR_SCREEN)
        instr_clear_screen(emu);
      else if(instr == INSTR_RETURN_SUB)
        ret_sub(emu);
      break;
   case 0x1:
      // jump
      instr_jmp(emu,  INSTR_NNN(instr));
      break;
   case 0x2:
      call_sub(emu, INSTR_NNN(instr));
      break;
   case 0x3:
      skip_eq(emu, emu->cpu->gp_r[INSTR_X(instr)], INSTR_NN(instr));
      break;
   case 0x4:
      skip_neq(emu, emu->cpu->gp_r[INSTR_X(instr)], INSTR_NN(instr));
      break;
   case 0x5:
      skip_eq(emu, emu->cpu->gp_r[INSTR_X(instr)], emu->cpu->gp_r[INSTR_Y(instr)]);
      break;
   case 0x6:
      // set register vx at NN value
      set_vreg(emu, INSTR_X(instr), INSTR_NN(instr));
      break;
   case 0x7:
      // add value NN to register vx, needs to modify flag register
      add_vreg(emu, INSTR_X(instr), INSTR_NN(instr));
      break;
   case 0x8:
      switch(INSTR_N(instr)) {
        case 0x0:
           set_xy(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x1:
           bitwise_or(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x2:
           bitwise_and(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x3:
           bitwise_xor(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x4:
           add_rrc(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x5:
           sub_rr(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x6:
           reg_rshift(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0x7:
           sub_rrrev(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
        case 0xE:
           reg_lshift(emu, INSTR_X(instr), INSTR_Y(instr));
           break;
      }
      break;
   case 0x9:
      // set register vx at NN value
      skip_neq(emu, emu->cpu->gp_r[INSTR_X(instr)], emu->cpu->gp_r[INSTR_Y(instr)]);
      break;
   case 0xA:
      // set ix to NNN
      set_ixreg(emu, INSTR_NNN(instr));
      break;
   case 0xB:
      instr_jmpoff(emu, INSTR_X(instr), INSTR_NNN(instr));
      break;
   case 0xC:
      instr_rand(emu, INSTR_X(instr), INSTR_NN(instr));
      break;
   case 0xD:
      // draw on screen
      draw_dp(emu, INSTR_X(instr), INSTR_Y(instr), INSTR_N(instr));
      break;
   case 0xE:
      switch(INSTR_NN(instr)) {
        case 0x9E:
          instr_skkeypr(emu, INSTR_X(instr));
          break;
        case 0xA1:
          instr_skkeynpr(emu, INSTR_X(instr));
          break;
      }
      break;
   case 0xF:
      switch(INSTR_NN(instr)) {
        case 0x07:
          instr_setvxdt(emu, INSTR_X(instr));
          break;
        case 0x0A:
          instr_getkey(emu, INSTR_X(instr));
          break;
        case 0x15:
          instr_setdt(emu, INSTR_X(instr));
          break;
        case 0x18:
          instr_setst(emu, INSTR_X(instr));
          break;
        case 0x1E:
          instr_addvxix(emu, INSTR_X(instr));
          break;
        case 0x29:
          instr_fontchar(emu, INSTR_X(instr));
          break;
        case 0x33:
          instr_bindec(emu, INSTR_X(instr));
          break;
        case 0x55:
          instr_stmem(emu, INSTR_X(instr));
          break;
        case 0x65:
          instr_ldmem(emu, INSTR_X(instr));
          break;
      }
      break;
   default:
      die("*** EMU ERROR: UNRECOGNIZED INSTRUCTION %hu ***", instr);
 }
 // decode
 // execute
 return 1;
}


// draw pixmap
__private void draw_pixmap(emu_s *emu) {
  // clear screen
  if(!SDL_SetRenderDrawColor(emu->dp->hw->rnd, OFF_COLOR_R, OFF_COLOR_G, OFF_COLOR_B, SDL_ALPHA_OPAQUE)) {
    SDL_Log("Couldn't draw color on screen : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  if(!SDL_RenderClear(emu->dp->hw->rnd)) {
    SDL_Log("Couldn't clear the screen : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  // redraw backbuffer
  if(!SDL_SetRenderDrawColor(emu->dp->hw->rnd, ON_COLOR_R, ON_COLOR_G, ON_COLOR_B, SDL_ALPHA_OPAQUE)) {
    SDL_Log("Couldn't set pixel color : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
  // get scaled pixel dimensions
  uint8_t      *pixmap  = emu->dp->pixmap;
  unsigned int xpix_dim = emu->dp->res_x / CHIP_DPW;
  unsigned int ypix_dim = emu->dp->res_y / CHIP_DPH;
  for(size_t y = 0; y < CHIP_DPH; y++) {
    for(size_t x = 0; x < CHIP_DPW; x++) {
      if(pixmap[PIXMAP_IDX(y, x)]) {
        SDL_FRect rect = {x * xpix_dim, y * ypix_dim, xpix_dim, ypix_dim};
        if(!SDL_RenderFillRect(emu->dp->hw->rnd, &rect)) {
          SDL_Log("Couldn't draw pixel : %s", SDL_GetError());
          die("*** EMU GRAPHICS ERROR ***");
        }
      }
    }
  }
  if(!SDL_RenderPresent(emu->dp->hw->rnd)) {
    SDL_Log("Couldn't present current renderer backbuffer : %s", SDL_GetError());
    die("*** EMU GRAPHICS ERROR ***");
  }
}

// cpu clock emulation


__private void poll_events(emu_s *emu, uint8_t *quit) {
  SDL_Event e;
  while (SDL_PollEvent(&e)){
    if(e.type == SDL_EVENT_QUIT)
      *quit = true;
    else if(e.type == SDL_EVENT_KEY_DOWN) {
      if(e.key.key == SDLK_ESCAPE) {
        *quit = true;
        break;
      }
      int8_t idx = get_keymap(e.key.key);
      if(idx != -1)
        emu->dp->keypad[idx].pressed = 1;
    }
    else if(e.type == SDL_EVENT_KEY_UP) {
      int8_t idx = get_keymap(e.key.key);
      if(idx != -1)
        emu->dp->keypad[idx].pressed = 0;
    }
  }
}



// remove unused
void emu_loop(emu_s *emu) {
  uint8_t quit = false;
  SDL_SetRenderDrawColor(emu->dp->hw->rnd, 0, 0, 0, 255);
  SDL_RenderClear(emu->dp->hw->rnd);
  SDL_RenderPresent(emu->dp->hw->rnd);
  // create SDL clock
  clock_s *dp_clk;
  init_clock(&dp_clk, DEFAULT_RFR);
  // start clocks
  clock_start(dp_clk);
  clock_start(emu->cpu->clock);
  while (!quit){
      // execute next instr
      exec_instr(emu);
      // poll keyboard events
      poll_events(emu, &quit);
      if(clock_exp(dp_clk)) {
        draw_pixmap(emu);
        // dec timers
        dec_timer(emu->cpu->s_timer);
        dec_timer(emu->cpu->d_timer);
        clock_start(dp_clk);
      }
      clock_sync(emu->cpu->clock);
  }
  free(dp_clk);
}





void free_emu(emu_s *emu) {
  free(emu->mem->mem_base);
  free(emu->mem);
  free(emu->cpu->s_timer);
  free(emu->cpu->d_timer);
  free(emu->cpu->clock);
  free(emu->cpu);
  display_free(emu->dp);
  free(emu);
}
