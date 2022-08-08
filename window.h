#ifndef _WINDOW_
#define _WINDOW_
#include<ncurses/ncurses.h>

typedef struct { int x, y; } coord;

typedef struct {
  WINDOW* pad;
  coord dimen;
  coord max;
  coord offset;
  int cur_pos;
} PAD;

PAD new_pad(int dimeny, int dimenx, int offsety, int offsetx);
PAD new_subpad(PAD parent_pad, int dimeny, int dimenx, int offsety, int offsetx);
int pad_rf(PAD pad);
int win_clr_pad_rf(PAD pad);

#endif