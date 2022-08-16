#ifndef _INIT_FUNCS_
#define _INIT_FUNCS_

#include "./todo.h"
#include "./string/string.h"

extern string TODO_FILE_PATH;
extern char* TODO_FILE_NAME;

todo* init_todos(int* todo_count);
void init_colors();

#endif // _INIT_FUNCS_
