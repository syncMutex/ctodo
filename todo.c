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
  WRITE_FILE(t.completed_date, char, 11, f);
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
  READ_FILE(t->completed_date, char, 11, f);

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

void get_today_date(int* dd, int* mm, int* yyyy) {
  time_t ti = time(NULL);
  struct tm time = *localtime(&ti);

  *dd = time.tm_mday;
  *mm = time.tm_mon + 1;
  *yyyy = time.tm_year + 1900;
}

void get_today_date_str(char str[11]) {
  int dd, mm, yyyy;
  get_today_date(&dd, &mm, &yyyy);
  sprintf(str, "%d-%d-%d", dd, mm, yyyy);
}

int date_diff(char* d1, char* d2) { 
  int diff = 0;
  int temp1 = 0, temp2 = 0;
  
  for(int i = 0; i <= 11; i++) {
    if(i < 11 && d1[i] == '\0' || d2[i] == '\0') break;

    if(d1[i] == '-' || d1[i] == '\0') {
      diff = temp1 - temp2;
      temp1 = 0;
      temp2 = 0;
      if(diff != 0) break;
    } else {
      temp1 = (temp1 * 10) + d1[i] - '0';
      temp2 = (temp2 * 10) + d2[i] - '0';
    }
  }
  
  if(diff < 0) diff *= -1;
  return diff; 
}

todo* create_todo(char* todo_string) {
  todo* newtodo = malloc(sizeof(todo));
  if(newtodo == NULL) return NULL;

  char cur_date[11];
  get_today_date_str(cur_date);

  strncpy(newtodo->created_date, cur_date, 10);
  strncpy(newtodo->modified_date, cur_date, 10);
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

bool delete_todo(todo** _todos, int todo_idx, int* todo_count) {
  int tcount = *todo_count;
  if(tcount <= 0) return false;

  todo* todos = *_todos;
  free(todos[todo_idx].todo.val);
  todo* new_todos;

  if(tcount != 1) {
    for(int i = todo_idx; i < tcount - 1; i++)
      todos[i] = todos[i + 1];

    new_todos = realloc(todos, sizeof(todo) * (tcount - 1));
    if(new_todos == NULL) return false;
  } else {
    new_todos = NULL;
  }

  *_todos = new_todos;
  (*todo_count)--;
  return true;
}

bool edit_todo(todo* t, char* new_todo_text) {
  set_str(&(t->todo), new_todo_text);

  char cur_date[11];
  get_today_date_str(cur_date);

  strncpy(t->modified_date, cur_date, 10);
  return true;
}

void toggle_complete_todo(todo* t) {
  t->is_completed = !(t->is_completed);

  if(t->is_completed) {
    char cur_date[11];
    get_today_date_str(cur_date);
    strncpy(t->completed_date, cur_date, 10);
  } else {
    for(int i = 0; i < 11; i++)
      t->completed_date[i] = '\0';
  }
}

void delete_old_todos(todo** _todos, int *todo_count) {
  todo* todos = *_todos;
  char cur_date[11];
  get_today_date_str(cur_date);

  for(int i = 0, tc = *todo_count; i < tc; i++) {
    todo t = todos[i];
    if(t.is_completed && (date_diff(cur_date, t.completed_date) > 2))
      delete_todo(_todos, i, todo_count);
  }
}

void print_todo(todo t) {
  printf("{ todo: \"%s\", is_completed: %d, created_date: \"%s\", modified_date: \"%s\" }\n", 
    t.todo.val, 
    t.is_completed,
    t.created_date,
    t.modified_date);
}