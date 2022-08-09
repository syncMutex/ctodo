#include<stdlib.h>
#include<string.h>
#include<ncurses/ncurses.h>

#if (defined(_WIN32) || defined(__WIN32__))

#include <direct.h>
#define mkdir(x, y) _mkdir(x)

#else

#include<sys/stat.h>
#include<sys/types.h>

#endif

#include "./todo.h"
#include "./todo-colors.h"

const char* TODO_FILE_NAME = "todos.bin";

todo* init_todos(int* todo_count) {
  #ifdef _WIN32
    const char* TODO_DIR_PATH = strcat(getenv("APPDATA"), "/ctodo");
  #else
    const char* TODO_DIR_PATH = strcat(getenv("HOME"), "/.local/share/ctodo");
  #endif

  string TODO_FILE_PATH = String(TODO_DIR_PATH, -1);

  str_cat_charptr(&TODO_FILE_PATH, "/");
  str_cat_charptr(&TODO_FILE_PATH, TODO_FILE_NAME);

  mkdir(TODO_DIR_PATH, 0700);

  *todo_count = 0;
  todo* todos = read_todos_from_file(TODO_FILE_PATH.val, todo_count);
  free(TODO_FILE_PATH.val);
  
  return todos;
}

void init_colors() {
  start_color();
  init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_RED);
  init_pair(COLOR_COMPLETE, COLOR_WHITE, COLOR_GREEN);
  init_pair(COLOR_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);
  init_pair(COLOR_MOVE_MODE, COLOR_BLACK, COLOR_YELLOW);
}