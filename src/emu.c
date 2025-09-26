#include <stdio.h>
#include <chip8.h>
#include <emu.h>

#define FILE_MAX    256



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
  if( opt[O_s].set ) clock_s = (uint16_t)(opt[O_s].value->ui);
  printf("DBG PROG NAME & CLOCK: %s %hu \n", prog_name, clock_s);
  emu_ctor(&emu, clock_s);
  load_rom(emu, prog_name);
  
  // EMU LOOP


  // END
  free(prog_name);
  free_emu(emu);
  return 0;
}
