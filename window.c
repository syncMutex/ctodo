#include<ncurses/ncurses.h>
#include<assert.h>

#include "window.h"

PAD new_pad(int dimeny, int dimenx, int offsety, int offsetx) {
  assert(offsetx >= 0 && offsety >= 0);
  PAD pad;

  if(dimenx == -1)
    dimenx = getmaxx(stdscr);
  if(dimeny == -1)
    dimeny = getmaxy(stdscr);

  pad.pad = newpad(dimeny, dimenx);
  pad.dimen.x = dimenx;
  pad.dimen.y = dimeny;
  pad.offset.x = offsetx;
  pad.offset.y = offsety;
  getmaxyx(stdscr, pad.max.y, pad.max.x);

  if(pad.max.x > pad.dimen.x) pad.max.x = pad.dimen.x;

  pad.cur_pos = 0;

  return pad;
}

PAD new_subpad(PAD ppad, int dimeny, int dimenx, int offsety, int offsetx) {
  assert(offsetx >= 0 && offsety >= 0);
  assert(dimeny + offsety <= ppad.dimen.y && dimenx + offsetx <= ppad.dimen.x);

  PAD spad;

  if(dimenx == -1)
    dimenx = ppad.dimen.x - offsetx;
  if(dimeny == -1)
    dimeny = ppad.dimen.y - offsety;

  spad.pad = subpad(ppad.pad, dimeny, dimenx, offsety, offsetx);
  spad.dimen.x = dimenx;
  spad.dimen.y = dimeny;
  spad.offset.x = offsetx;
  spad.offset.y = offsety;
  spad.max.y = ppad.max.y;
  spad.max.x = ppad.max.x;

  spad.cur_pos = 0;

  return spad;
}

int pad_rf(PAD pad) {
  // pad.max.y - 1 [because getmaxyx gives line count and not max idx]
  int my = pad.max.y - 1 - pad.offset.y;
  int mx = pad.max.x - pad.offset.x;
  return prefresh(
    pad.pad,
    pad.cur_pos, 0,
    pad.offset.y, pad.offset.x,
    my, mx);
}

int win_clr_pad_rf(PAD pad) {
  clear();
  refresh();
  return pad_rf(pad);
}
