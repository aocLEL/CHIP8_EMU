#include <stdio.h>
#include <chip8.h>
#include <emu.h>

#define FILE_MAX    256
#define WIN_NAME    "CHIP8 Emulator"


// keyboard
// l'utente da riga di comando tramite l'opzione keys puó passare un array di 15 tasti personalizzati, altrimenti si usano quelli di default
// i tasti vengono memorizzati nella struttura display, dove vengono associati ai relativi eventi SDL, la struttura contiene un campo keypad array di 15 strutture chip_key_s, ciascuna contenente 2 membri: il nome SDL del tasto e isset 
// quando un tasto viene premuto , il suo relativo campo set nel keypad viene messo 1 , quando viene rilasciato a 0
// il keycode SDL non é altro che il valore unicode del carattere. rendendo disponibili per il keybad solo quelli alfanumerici (isalpha) non avremo problemi


// ricorda di aggiungere handler per SIGTERM
// ricorda di scrivere emu_die, come funzione che chiude l'intero emulatore in caso di qualunque problema e dealloca tutto ció che era eventualmente stato allocato, mettila al posto di tutte le sequenze fprintf/free/exit
// ricorda di implementare il beep sound del sound timer

// importare parser opzioni
int main(int argc, char **argv) {
  char *prog_name = NULL;
  __argv option_s* opt = argv_parse(PROG_OPT, argc, argv, &prog_name); // NOLINT
	if( opt[O_H].set ) argv_usage(opt, argv[0]);
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
  unsigned int res_x          = opt[O_w].set ? (unsigned int)(opt[O_w].value->ui) : DEFAULT_REAL_DPW;
  unsigned int res_y          = opt[O_h].set ? (unsigned int)(opt[O_h].value->ui) : DEFAULT_REAL_DPH;
  unsigned int ref_rt         = opt[O_r].set ? (unsigned int)(opt[O_r].value->ui) : DEFAULT_RFR;
  optval_u     *raw_keypad    = DEFAULT_KEYPAD;
  if(opt[O_k].set) {
    if(opt[O_k].set != CHIP_KEYPAD_SIZE) {
      fprintf(stderr, "You have to specify all %d alphanumeric keypad keys!!!\n", CHIP_KEYPAD_SIZE);
      free(prog_name);
      exit(EXIT_FAILURE);
    }
    raw_keypad = opt[O_k].value;
  }

  // emu constructor
  emu_ctor(&emu, clock_s, full_win_name, res_x, res_y, ref_rt, opt[O_S].set, raw_keypad);
  load_rom(emu, prog_name);  
  // EMU LOOP
  emu_loop(emu);

  // END
  free(prog_name);
  free_emu(emu);
  return 0;
}
