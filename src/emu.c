#include <stdio.h>
#include <chip8.h>

// importare parser opzioni
int main(int argc, char **argv) {
  emu_s *emu;
  init_emu(emu, prog_name, clock_s);
  printf("Hello world\n");
  return 0;
}
