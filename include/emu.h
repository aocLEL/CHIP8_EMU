#ifndef __EMU_H__
#define __EMU_H__


#include "opt.h"
#include "chip8.h"

// AVAILABLE OPT
option_s PROG_OPT[] = {
  {'s', "--clockspeed"     , "Set specific clock speed"                                           , OPT_NUM            , 0, 0},
	{'h', "--help"           , "display this"                                                       , OPT_END | OPT_NOARG, 0, 0}
};


// https://www.sco.com/developers/gabi/latest/ch4.eheader.html#elfid

// opt ascii indexes in the opt array
typedef enum{
  O_s,
	O_h
}OPT_E;




#endif
