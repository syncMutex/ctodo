#include<stdlib.h>
#include<ncurses/ncurses.h>

#include "./string/string.h"
#include "./todo.h"
#include "./todo-colors.h"
#include "./window.h"
#include "./init-funcs.h"

const char* TODO_FILE_NAME = "todos.bin";
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
PAD top_bar;

#define tline_count(tidx) ((todos[tidx].todo.length + 3) / main_pad.max.x + 1)
#define pad_clear(p) wclear(p.pad)
#define tlnc_wlnbr(tidx) (tline_count(tidx) + 1)
#define no_change cur_tidx
#define update_todo_curs(pos, do_paint) _update_todo_curs(pos, do_paint, is_move_mode)
#define cur_tidx todo_cursor.cur_todo_idx
#define KEY_ESC 27

#define wprint_and_update_curs(_pad, str, curs_pos, str_idx) for(int i = str_idx; i < str.length; i++)\
  if(mvwaddch(_pad.pad, curs_pos.y, curs_pos.x++, str.val[i]) == ERR) {\
    curs_pos.y++;\
    curs_pos.x = 0;\
    i--;\
    continue;\
  }

void wprint_str_from(PAD pad, string str, coord curs_pos, int str_idx) {
  wmove(pad.pad, curs_pos.y, curs_pos.x);
  wclrtobot(pad.pad);
  wprint_and_update_curs(pad, str, curs_pos, str_idx);
}

string get_new_input(char* init_str, char* placeholder) {
  PAD input_pad = new_subpad(main_pad, -1, -1, main_pad.offset.y + 2, 1);

  int pad_cur_pos = main_pad.cur_pos;
  main_pad.cur_pos = 0;

  clear();
  pad_clear(main_pad);
  waddstr(main_pad.pad, placeholder);

  string str = String(init_str, -1);
  coord curs_pos = {.x = 0, .y = 0};
  int str_idx = 0;

  curs_set(1);
  win_clr_pad_rf(main_pad);
  wprint_and_update_curs(input_pad, str, curs_pos, str_idx);
  str_idx = str.length;
  pad_rf(input_pad);

  #define quit_loop() quit = true; break
  #define check_left_bounds() if(curs_pos.x < 0){curs_pos.x = input_pad.dimen.x - 1;curs_pos.y--;}
  #define check_right_bounds() if(curs_pos.x > input_pad.dimen.x){curs_pos.x = 1;curs_pos.y++;}

  #define go_left() curs_pos.x--; str_idx--; check_left_bounds()
  #define go_right() curs_pos.x++; str_idx++; check_right_bounds()

  bool quit = false;
  int c;
  while(!quit) {
    c = getch();
    switch(c) {
    case KEY_ESC:
      set_str(&str, "");
      quit_loop();
    case KEY_LEFT:
      if(str_idx <= 0) break;
      go_left();
      break;
    case KEY_RIGHT:
      if(str_idx >= str.length) break;
      go_right();
      break;
    case 10:
    case KEY_ENTER:
      quit_loop();
    case 8:
    case 127:
    case KEY_BACKSPACE:
      if(str_idx - 1 < 0) break;
      if(!remove_char(&str, str_idx - 1)) {
        printf("error while pop_char\n");
        quit_loop();
      }
      go_left();
      wprint_str_from(input_pad, str, curs_pos, str_idx);
      break;
    default:
      if(!insert_char(&str, c, str_idx)) {
        printf("error while insert_char\n");
        quit_loop();
      }
      wprint_str_from(input_pad, str, curs_pos, str_idx);
      go_right();
    }
    wmove(input_pad.pad, curs_pos.y, curs_pos.x);
    pad_rf(input_pad);
  }

  curs_set(0);
  delwin(input_pad.pad);

  main_pad.cur_pos = pad_cur_pos;

  return str;
}

todo* get_new_todo_input() {
  string str = get_new_input("", "New todo\n\n");
  todo* t = NULL;
  if(str.length != 0)
    t = create_todo(str.val);
  free(str.val);
  return t;
}

bool modify_todo(todo* t) {
  if(t == NULL) return false;
  string str = get_new_input(t->todo.val, "Edit todo\n\n");

  bool can_save = true;

  can_save = str.length != 0 ? edit_todo(t, str.val) : false;
     
  free(str.val);

  return can_save;
}

void view_cur_todo_details() {
  todo t = todos[cur_tidx];
  int dimenx = main_pad.max.x - 1 < 80 ? main_pad.max.x - 2 : 80;
  int max_cur_pos = 7 + ((t.todo.length + 6) / dimenx + 1);

  PAD todo_details_pad = new_subpad(main_pad, max_cur_pos, dimenx, 1, 1);

  clear();
  pad_clear(main_pad);

  wmove(todo_details_pad.pad, 0, 0);
  wprintw(todo_details_pad.pad,
    "todo: %s\n\ncreated on: %s\n\nmodified on: %s\n\n",
    t.todo.val, t.created_date, t.modified_date);

  enum todo_colors att = COLOR_PAIR(t.is_completed ? COLOR_COMPLETE_TEXT : COLOR_PENDING_TEXT);

  waddstr(todo_details_pad.pad, "status: ");
  wattron(todo_details_pad.pad, att);
  waddstr(todo_details_pad.pad, t.is_completed ? "completed" : "pending");
  wattroff(todo_details_pad.pad, att);
  
  win_clr_pad_rf(todo_details_pad);
  
  int c = -1;
  while(c != 27 && c != 10 && c != KEY_ENTER) {
    c = getch();
    switch(c) {
    case KEY_DOWN:
    case 'j':
      if(todo_details_pad.cur_pos >= max_cur_pos - 2) break;
      todo_details_pad.cur_pos++;
      win_clr_pad_rf(todo_details_pad);
      break;
    case KEY_UP:
    case 'k':
      if(todo_details_pad.cur_pos <= 0) break;
      todo_details_pad.cur_pos--;
      win_clr_pad_rf(todo_details_pad);
      break;
    }
  }
  delwin(todo_details_pad.pad);
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

  for(int i = 0; i < todo_idx; i++)
    sum += tlnc_wlnbr(i);
  
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

void _update_todo_curs(int prevtidx, bool do_paint, bool is_move_mode) {
  if(todos == NULL) return;
  int curtidx = cur_tidx;

  todo prev_todo = todos[prevtidx];
  todo to_hilgt_todo = todos[curtidx];

  enum todo_colors ch = COLOR_PAIR(is_move_mode ? COLOR_MOVE_MODE : COLOR_HIGHLIGHT);

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

void resize_main_pad_if_needed() {
  if(todo_cursor.max_offset >= main_pad.dimen.y) {
    main_pad.dimen.y = todo_cursor.max_offset;
    wresize(main_pad.pad, main_pad.dimen.y, main_pad.dimen.x);
  }
}

void check_pad_space_add_todo(todo* new_todo) {
  if(new_todo != NULL)
    add_todo(&todos, &todo_count, new_todo);
  else return;

  todo_cursor.max_offset += tlnc_wlnbr(todo_count - 1);
  resize_main_pad_if_needed();
}

void swap_todos(int idx1, int idx2) {
  todo temp = todos[idx1];
  todos[idx1] = todos[idx2];
  todos[idx2] = temp;
}

void clrnln_from(int from, int n) {
  for(int i = 0; i < n; i++, from++) {
    wmove(main_pad.pad, from, 0);
    wclrtoeol(main_pad.pad);
  }
}

void test_todos() {
  string s = String("", -1);

  int c = 97;

  for(int i = 0; i < 500; i++, c++) {
    if(c == 97 + 26) c = 97;
    append_char(&s, c);
  }

  for(int i = 0; i < 15; i++) {
    check_pad_space_add_todo(create_todo(s.val));
    for(int j = 0; j < 30; j++) pop_char(&s);
  }

  free(s.val);
}

void update_top_bar(bool can_save) {
  pad_clear(top_bar);
  if(can_save) {
    wattron(top_bar.pad, COLOR_PAIR(COLOR_PENDING));
    mvwprintw(top_bar.pad, 0, 0, "you have unsaved changes");
    wattroff(top_bar.pad, COLOR_PAIR(COLOR_PENDING));
  } else {
    wattron(top_bar.pad, COLOR_PAIR(COLOR_COMPLETE));
    mvwprintw(top_bar.pad, 0, 0, "saved changes");
    wattroff(top_bar.pad, COLOR_PAIR(COLOR_COMPLETE));
  }
  pad_rf(top_bar);
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

  int maxx = getmaxx(stdscr);
  main_pad = new_pad(-1, maxx - 1, 2, 1);
  top_bar = new_pad(1, -1, 0, 0);

  refresh();

  todos = init_todos(&todo_count, TODO_FILE_NAME);

  if(todo_count == 0) {
    waddstr(main_pad.pad, "Man make some todos...");
    win_clr_pad_rf(main_pad);
  } else {
    todo_cursor.max_offset = get_offset(todo_count);
    resize_main_pad_if_needed();
    render_todos();
  }

  todo_cursor.offset = get_offset(0);
  todo_cursor.screen_bounds.upr = main_pad.offset.y;
  // because getmaxyx gives line count and not max idx
  todo_cursor.screen_bounds.lwr = main_pad.max.y - 1 - main_pad.offset.y;

  bool quit = false;
  bool is_move_mode = false;
  bool can_save = false;
  int prev_c;

  if(todo_count != 0)
    update_todo_curs(0, true);

  #define quit_loop() quit = true; break

  #define set_can_save(to) can_save = to; update_top_bar(can_save)

  while(!quit) {
    int c = getch();

    if(is_move_mode && (c == 'n' || c == 'i' || c == 'x' || c == 'd' || c == 'o')) continue;

    if(todo_count == 0 && (c != 'n' && c != 'q' && c != 'w')) continue;

    switch(c) {
    case 'q':
      quit_loop();
    case 'n':;
      todo* new_todo = get_new_todo_input();
      
      if(new_todo == NULL) {
        render_todos();
        update_todo_curs(no_change, true);
        break;
      }

      check_pad_space_add_todo(new_todo);
      render_todos();
      
      if(todo_count <= 2) {
        cur_tidx = todo_count - 1;
        todo_cursor.offset = get_offset(cur_tidx);
        update_todo_curs(no_change, true);
      } else {
        while(cur_tidx != todo_count - 2)
          update_todo_curs(cur_tidx++, false);
        update_todo_curs(cur_tidx++, true);
      }

      set_can_save(true);

      break;
    case 'i':
      can_save = modify_todo(&(todos[cur_tidx]));
      render_todos();
      update_todo_curs(no_change, true);
      update_top_bar(can_save);
      break;
    case 'o':
      view_cur_todo_details();
      render_todos();
      update_todo_curs(no_change, true);
      break;
    case KEY_DOWN:
    case 'j':
      if(cur_tidx >= todo_count - 1) break;
      if(is_move_mode) {
        set_can_save(true);
        swap_todos(cur_tidx, cur_tidx + 1);
        clrnln_from(todo_cursor.offset, tlnc_wlnbr(cur_tidx) + tlnc_wlnbr(cur_tidx + 1));
      }
      update_todo_curs(cur_tidx++, true);
      break;
    case KEY_UP:
    case 'k':
      if(cur_tidx <= 0) break;
      if(is_move_mode) {
        set_can_save(true);
        clrnln_from(todo_cursor.offset - tlnc_wlnbr(cur_tidx - 1), tlnc_wlnbr(cur_tidx) + tlnc_wlnbr(cur_tidx - 1));
        todo_cursor.offset -= tlnc_wlnbr(cur_tidx - 1) - tlnc_wlnbr(cur_tidx);
        swap_todos(cur_tidx, cur_tidx - 1);
      }
      update_todo_curs(cur_tidx--, true);
      break;
    case 'x':
      todos[cur_tidx].is_completed = !(todos[cur_tidx].is_completed);
      render_todo(todo_cursor.offset, 0, todos[cur_tidx], COLOR_PAIR(COLOR_HIGHLIGHT));
      pad_rf(main_pad);
      set_can_save(true);
      break;
    case 'm':
      is_move_mode = !is_move_mode;
      update_todo_curs(no_change, true);
      break;
    case 10:
    case KEY_ENTER:
      if(!is_move_mode) break;
    case KEY_ESC:
      is_move_mode = false;
      update_todo_curs(no_change, true);
      break;
    case 'w':
      if(!can_save) break;
      if(!write_todos_to_file(TODO_FILE_PATH.val, todos, todo_count)) {
        endwin();
        printf("error while writing todos to file.\n");
        quit_loop();
      }
      set_can_save(false);
      break;
    case 'd':
      if(prev_c == 'd') {
        if(todo_count == 0) break;

        delete_todo(&todos, cur_tidx, &todo_count);
        render_todos();

        if(cur_tidx >= todo_count)
          update_todo_curs(cur_tidx--, true);
        else
          update_todo_curs(cur_tidx, true);

        if(todo_count == 0) {
          pad_clear(main_pad);
          waddstr(main_pad.pad, "Man make some todos...");
          win_clr_pad_rf(main_pad);
        }

        set_can_save(true);
        c = -1;
      }
    }
    prev_c = c;
  }

  for(int i = 0; i < todo_count; i++) free(todos[i].todo.val);
  free(todos);
  free(TODO_FILE_PATH.val);
  
  delwin(main_pad.pad);
  endwin();
  return 0;
}
