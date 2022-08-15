#ifndef _INIT_FUNCS_
#define _INIT_FUNCS_

#include "./todo.h"
#include "./string/string.h"

string TODO_FILE_PATH;

todo* init_todos(int* todo_count, const char* file_name);
void init_colors();

#endif // _INIT_FUNCS_
