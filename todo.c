#include "./todo.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "./string/string.h"
#include<time.h>

#define WRITE_FILE(field, data_type, size, file) if(fwrite(field, sizeof(data_type), size, f) != size) return false
#define READ_FILE(field, data_type, size, file) if(fread(field, sizeof(data_type), size, f) != size) return false

bool write_todo(todo t, FILE* f) {
  WRITE_FILE(t.created_date, char, 11, f);
  WRITE_FILE(t.modified_date, char, 11, f);
  WRITE_STRING(t.todo, f);
  WRITE_FILE(&(t.is_completed), bool, 1, f);
  return true;
}

string* read_string(FILE* f) {
  size_t len = 0;
  if(fread(&len, sizeof(size_t), 1, f) != 1) return NULL;
  char* str_buf = malloc(len + 1);

  if(fread(str_buf, sizeof(char), len, f) != len) {
    free(str_buf);
    return NULL;
  }

  str_buf[len] = '\0';
  string s = String(str_buf, -1);
  string* strptr = &s;
  free(str_buf);
  return strptr;
}

bool read_todo(todo* t, FILE* f) {
  READ_FILE(t->created_date, char, 11, f);
  READ_FILE(t->modified_date, char, 11, f);

  string* temps = read_string(f);
  if(temps == NULL) return false;
  t->todo = *temps;

  READ_FILE(&(t->is_completed), bool, 1, f);

  return true;
}

bool write_todos_to_file(const char* file_name, todo* todos, int todo_count) {
  FILE* f = fopen(file_name, "wb");

  if(f == NULL) return false;

  if(fwrite(&todo_count, sizeof(int), 1, f) != 1) return false;

  for(int i = 0; i < todo_count; i++)
    if(!write_todo(todos[i], f)) return false;

  if(fclose(f) == EOF) return false;
  
  return true;
}

todo* read_todos_from_file(const char* file_name, int* todo_count) {
  FILE* f = fopen(file_name, "rb");
  
  if(f == NULL) {
    f = fopen(file_name, "wb");
    fclose(f);
    return NULL;
  }

  if(fread(todo_count, sizeof(int), 1, f) != 1) 
    return NULL; 
  
  todo* todos = malloc(sizeof(todo) * *todo_count);

  for(int i = 0; i < *todo_count; i++)
    if(!read_todo(&todos[i], f)) {
      free(todos);
      return NULL;
    }

  if(fclose(f) == EOF) {
    free(todos);
    return NULL;
  }
  
  return todos;
}

todo* create_todo(char* todo_string) {
  todo* newtodo = malloc(sizeof(todo));
  if(newtodo == NULL) return NULL;

  time_t t = time(NULL); 
  struct tm time = *localtime(&t);
  char created_date[11];

  sprintf(created_date, "%d-%d-%d", time.tm_mday, time.tm_mon + 1, time.tm_year + 1900);

  strncpy(newtodo->created_date, created_date, 10);
  strncpy(newtodo->modified_date, created_date, 10);
  newtodo->todo = String(todo_string, -1);
  newtodo->is_completed = false;

  return newtodo;
}

bool add_todo(todo** todos, int *todo_count, todo* new_todo) {
  todo* new_todos;

  new_todos = realloc(*todos, sizeof(todo) * (*todo_count + 1));

  if(new_todos == NULL) return false;
  new_todos[*todo_count] = *new_todo;
  *todos = new_todos;
  (*todo_count)++;
  return true;
}

void print_todo(todo t) {
  printf("{ todo: \"%s\", is_completed: %d, created_date: \"%s\", modified_date: \"%s\" }\n", 
    t.todo.val, 
    t.is_completed,
    t.created_date,
    t.modified_date);
}