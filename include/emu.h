#ifndef __EMU_H__
#define __EMU_H__


#include "opt.h"
#include "chip8.h"

// AVAILABLE OPT
option_s PROG_OPT[] = {
  {'s', "--clockspeed"     , "Set specific clock speed"                                           , OPT_NUM            , 0, 0},
  {'h', "--displayheight"  , "Set specific display height"                                        , OPT_NUM            , 0, 0},
  {'w', "--displaywidth"   , "Set specific display width"                                         , OPT_NUM            , 0, 0},
  {'r', "--refresh"        , "Set specific display refresh rate"                                  , OPT_NUM            , 0, 0},
  {'S', "--superchip"      , "Set CHIP version to SUPER-CHIP"                                     , OPT_NOARG          , 0, 0},
  {'k', "--keypad"         , "Set custom CHIP keypad"                                             , OPT_ARRAY | OPT_STR, 0, 0},
	{'H', "--help"           , "display this"                                                       , OPT_END | OPT_NOARG, 0, 0}
};



typedef enum{
  O_s,
  O_h,
  O_w,
  O_r,
  O_S,
  O_k,
	O_H
}OPT_E;




#endif
