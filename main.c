#include<stdlib.h>
#include<ncurses/ncurses.h>

#include "./string/string.h"
#include "./todo.h"
#include "./todo-colors.h"
#include "./window.h"
#include "./init-funcs.h"

todo* todos = NULL;
int todo_count = 0;

struct {
  int cur_todo_idx;
  int offset;
  int max_offset;
  
  struct {
    int upr;
    int lwr;
  } screen_bounds;
} todo_cursor;

void print_tcurs() {
  printf("{cur_tidx: %d, offset: %d, max_offset: %d, sc_bnds: {upr: %d, lwr: %d}}\n",
    todo_cursor.cur_todo_idx,
    todo_cursor.offset,
    todo_cursor.max_offset,
    todo_cursor.screen_bounds.upr,
    todo_cursor.screen_bounds.lwr);
}

PAD main_pad;

#define tline_count(tidx) ((todos[tidx].todo.length + 3) / main_pad.max.x + 1)
#define pad_clear(p) wclear(p.pad)
#define tlnc_wlnbr(tidx) (tline_count(tidx) + 1)

string get_new_input(char* placeholder) {
  PAD input_pad = new_subpad(main_pad, -1, -1, 3, 1);

  int pad_cur_pos = main_pad.cur_pos;
  main_pad.cur_pos = 0;

  clear();
  pad_clear(main_pad);
  wprintw(main_pad.pad, "%s", placeholder);

  bool quit = false;
  int c;
  coord MIN_CURS_POS = {.x = 0, .y = 0};
  coord curs_pos = {.x = MIN_CURS_POS.x, .y = MIN_CURS_POS.y};
  string str = String("", -1);

  curs_set(1);
  mvwaddch(main_pad.pad, curs_pos.y, curs_pos.x - 1, ' ');
  win_clr_pad_rf(main_pad);

  #define quit_loop() quit = true; break

  while(!quit) {
    c = getch();
    switch(c) {
    case KEY_LEFT:
      if(curs_pos.x <= MIN_CURS_POS.x) break;
      curs_pos.x--;
      wmove(main_pad.pad, curs_pos.y, curs_pos.x);
      break;
    case KEY_RIGHT:
      if(curs_pos.x >= str.length - 1) break;
      wmove(main_pad.pad, curs_pos.y, ++curs_pos.x);
      break;
    case 10:
    case KEY_ENTER:
      quit_loop();
    case 8:
    case 127:
    case KEY_BACKSPACE:
      if(curs_pos.x <= MIN_CURS_POS.x) break;
      if(!pop_char(&str)) {
        printf("error while pop_char\n");
        quit_loop();
      }
      curs_pos.x--;
      break;
    default:
      if(str.length == curs_pos.x - MIN_CURS_POS.x) {
        if(!append_char(&str, c)) {
          printf("error while _char\n");
          quit_loop();
        }
      } else {
        if(!insert_char(&str, c, curs_pos.x - MIN_CURS_POS.x)) {
          printf("error while insert_char\n");
          quit_loop();
        }
      }
      curs_pos.x++;
    }

    pad_clear(input_pad);
    mvwaddstr(input_pad.pad, MIN_CURS_POS.y, MIN_CURS_POS.x, str.val);
    pad_rf(input_pad);
  }

  curs_set(0);
  delwin(input_pad.pad);

  main_pad.cur_pos = pad_cur_pos;

  return str;
}

todo* get_new_todo_input() {
  string str = get_new_input("New todo\n\n");
  todo* t = NULL;
  if(str.length != 0)
    t = create_todo(str.val);
  free(str.val);
  return t;
}

void print_strike_through(char* str, int y, int x) {
  #ifdef _WIN32
    wattron(main_pad.pad, A_DIM);
    mvwaddstr(main_pad.pad, y, x, str);
    wattroff(main_pad.pad, A_DIM);
  #else
    wmove(main_pad.pad, y, x);
    for(int i = 0; str[i] != '\0'; i++) {
      waddch(main_pad.pad, str[i]);
      waddstr(main_pad.pad, "\u0336");
    }
  #endif
}

void render_todo(int y, int x, todo t, int opt_attr) {
  enum todo_colors c = COLOR_PAIR(t.is_completed ? COLOR_COMPLETE : COLOR_PENDING);

  wattron(main_pad.pad, c);
  mvwaddstr(main_pad.pad, y, x, "  ");
  wattroff(main_pad.pad, c);

  x += 3;

  if(opt_attr != -1) wattron(main_pad.pad, opt_attr);  
  t.is_completed ? print_strike_through(t.todo.val, y, x) : mvwaddstr(main_pad.pad, y, x, t.todo.val);
  if(opt_attr != -1) wattroff(main_pad.pad, opt_attr);  
}

void render_todos() {
  pad_clear(main_pad);

  coord print_pos = {0, 0};

  for(int i = 0; i < todo_count; i++) {
    todo t = todos[i];

    render_todo(print_pos.y, print_pos.x, t, -1);
    
    print_pos.x = 0;
    print_pos.y += tlnc_wlnbr(i);
  }
  win_clr_pad_rf(main_pad);
}

int get_offset(int todo_idx) {
  int sum = 0;

  for(int i = 0; i < todo_idx; i++) {
    int tlen = todos[i].todo.length;
    sum += 1 + (tlen / main_pad.max.x);
  }
  
  return sum;
}

void adjust_scr_bounds(int cur_end_offset) {
  int diff = 0;

  if(cur_end_offset > todo_cursor.screen_bounds.lwr) {
    diff = cur_end_offset - todo_cursor.screen_bounds.lwr;
  } else if(todo_cursor.offset <= todo_cursor.screen_bounds.upr) {
    diff = todo_cursor.offset - todo_cursor.screen_bounds.upr;
  }

  if(diff != 0) {
    main_pad.cur_pos += diff;
    todo_cursor.screen_bounds.lwr += diff;
    todo_cursor.screen_bounds.upr += diff;
  }
}

void update_todo_curs(int prevtidx, bool do_paint) {
  if(todos == NULL) return;
  int curtidx = todo_cursor.cur_todo_idx;

  todo prev_todo = todos[prevtidx];
  todo to_hilgt_todo = todos[curtidx];

  enum todo_colors ch = COLOR_PAIR(COLOR_HIGHLIGHT);

  if(prevtidx != curtidx) {
    if(prevtidx < todo_count)
      render_todo(todo_cursor.offset, 0, prev_todo, -1);

    if(curtidx > prevtidx)
      todo_cursor.offset += tlnc_wlnbr(prevtidx);
    else
      todo_cursor.offset -= tlnc_wlnbr(curtidx);

    int cur_end_offset = tline_count(curtidx) + todo_cursor.offset + main_pad.offset.y;

    adjust_scr_bounds(cur_end_offset);
  }

  if(do_paint) {
    render_todo(todo_cursor.offset, 0, to_hilgt_todo, ch);
    pad_rf(main_pad);
  }
}

void check_pad_space_add_todo(todo* new_todo) {
  if(new_todo != NULL)
    add_todo(&todos, &todo_count, new_todo);
  else return;

  todo_cursor.max_offset += tlnc_wlnbr(todo_count - 1);

  if(todo_cursor.max_offset >= main_pad.dimen.y) {
    main_pad.dimen.y = todo_cursor.max_offset;
    wresize(main_pad.pad, main_pad.dimen.y, main_pad.dimen.x);
  }
}

int main() {
  initscr();

  if(has_colors() == FALSE) {
    endwin();
    printf("man no colors :(\n");
    return -1;
  }

  init_colors();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);

  main_pad = new_pad(-1, -1, 1, 1);

  refresh();

  todos = init_todos(&todo_count);

  string s = String("", -1);

  for(int i = 0; i < 500; i++)
    append_char(&s, 'a');

  for(int i = 0; i < 15; i++) {
    check_pad_space_add_todo(create_todo(s.val));
    for(int j = 0; j < 30; j++) pop_char(&s);
  }

  free(s.val);
  
  if(todo_count == 0) {
    wprintw(main_pad.pad, "Man make some todos...");
    win_clr_pad_rf(main_pad);
  } else {
    render_todos();
  }

  todo_cursor.offset = get_offset(0);
  todo_cursor.screen_bounds.upr = main_pad.offset.y;
  // because getmaxyx gives line count and not max idx
  todo_cursor.screen_bounds.lwr = main_pad.max.y - 1 - main_pad.offset.y;
  update_todo_curs(0, true);

  bool quit = false;
  int prev_c;
  while(!quit) {
    int c = getch();

    switch(c) {
    case 'q':
      quit = true;
      break;
    case 'n':;
      todo* new_todo = get_new_todo_input();
      check_pad_space_add_todo(new_todo);
      render_todos();
      while(todo_cursor.cur_todo_idx != todo_count - 2)
        update_todo_curs(todo_cursor.cur_todo_idx++, false);

      update_todo_curs(todo_cursor.cur_todo_idx++, true);
      break;
    case KEY_DOWN:
    case 'j':
      if(todo_cursor.cur_todo_idx >= todo_count - 1) break;
      update_todo_curs(todo_cursor.cur_todo_idx++, true);
      break;
    case KEY_UP:
    case 'k':
      if(todo_cursor.cur_todo_idx <= 0) break;
      update_todo_curs(todo_cursor.cur_todo_idx--, true);
      break;
    case 'x':
      todos[todo_cursor.cur_todo_idx].is_completed = !(todos[todo_cursor.cur_todo_idx].is_completed);
      render_todo(todo_cursor.offset, 0, todos[todo_cursor.cur_todo_idx], COLOR_PAIR(COLOR_HIGHLIGHT));
      pad_rf(main_pad);
    case 'd':
      if(prev_c == 'd') {
        if(todo_count == 0) break;

        delete_todo(&todos, todo_cursor.cur_todo_idx, &todo_count);
        render_todos();

        if(todo_cursor.cur_todo_idx >= todo_count)
          update_todo_curs(todo_cursor.cur_todo_idx--, true);
        else
          update_todo_curs(todo_cursor.cur_todo_idx, true);

        if(todo_count == 0) {
          pad_clear(main_pad);
          wprintw(main_pad.pad, "Man make some todos...");
          win_clr_pad_rf(main_pad);
        }

        c = -1;
      }
    }
    prev_c = c;
  }

  for(int i = 0; i < todo_count; i++) free(todos[i].todo.val);
  free(todos);
  
  delwin(main_pad.pad);
  endwin();
  return 0;
}
