#include <stdio.h>
#include <chip8.h>
#include <emu.h>

#define FILE_MAX    256
#define WIN_NAME    "CHIP8 Emulator"


// importare parser opzioni
int main(int argc, char **argv) {
  char *prog_name = NULL;
  __argv option_s* opt = argv_parse(PROG_OPT, argc, argv, &prog_name); // NOLINT
	if( opt[O_h].set ) argv_usage(opt, argv[0]);
  if(!prog_name) {
    puts("No ROM given!! try again");
    exit(EXIT_FAILURE);
  }
  emu_s *emu;
  uint16_t clock_s = CLOCK_DEFAULT;
  char full_win_name[FILE_MAX];
  sprintf(full_win_name, "%s -- %s", WIN_NAME, prog_name);
  if( opt[O_s].set ) clock_s = (uint16_t)(opt[O_s].value->ui);
  printf("DBG PROG NAME & CLOCK: %s %hu \n", prog_name, clock_s);
  // setting display info
  unsigned int res_x  = opt[O_w].set ? (unsigned int)(opt[O_w].value->ui) : DEFAULT_REAL_DPW;
  unsigned int res_y  = opt[O_h].set ? (unsigned int)(opt[O_h].value->ui) : DEFAULT_REAL_DPH;
  unsigned int ref_rt = opt[O_r].set ? (unsigned int)(opt[O_r].value->ui) : DEFAULT_RFR;
  // emu constructor
  emu_ctor(&emu, clock_s, full_win_name, res_x, res_y, ref_rt);
  load_rom(emu, prog_name);  
  // EMU LOOP
  emu_loop(emu);

  // END
  free(prog_name);
  free_emu(emu);
  return 0;
}
