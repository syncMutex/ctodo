#ifndef _TODO_
#define _TODO_

#include<stdbool.h>
#include "string/string.h"

typedef struct {
  char created_date[11];
  char modified_date[11];
  bool is_completed;
  string todo;
} todo;

todo* read_todos_from_file(const char* file_name, int* todo_count);
bool write_todos_to_file(const char* file_name, todo* todos, int todo_count);

todo* create_todo(char* todo_string);
bool add_todo(todo** todos, int *todo_count, todo* new_todo);
bool delete_todo(todo** todos, int todo_idx, int* todo_count);
void print_todo(todo t);

#endif