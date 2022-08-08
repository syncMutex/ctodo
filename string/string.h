#ifndef _STRING_
#define _STRING_

#include<stdbool.h>
#include<stddef.h>

#define WRITE_STRING(str, file) \
  if(fwrite(&str.length, sizeof(size_t), 1, file) != 1) return false;\
  if(fwrite(str.val, sizeof(char), str.length, file) != str.length) return false

typedef struct {
  size_t capacity;
  size_t length;
  char* val;
} string;

string String(const char* s, size_t capacity);
bool set_str(string* str, char* new_string);
bool insert_char(string* str, char c, int idx);
bool append_char(string* str, char c);
bool pop_char(string* str);
bool str_cat_str(string* dest, string* src);
bool str_cat_charptr(string* dest, const char* str);
void print_string(string str);

#endif // _STRING_
