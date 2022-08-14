#include<stdio.h>
#include "./string.h"
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>

string String(const char* init_string, size_t capacity) {
  size_t length = strlen(init_string);
  
  if(capacity == -1) capacity = length;

  assert(length <= capacity);

  char* s = malloc((sizeof(char) * capacity) + 1);
  strcpy(s, init_string);
  s[length] = '\0';

  string str = {
    .capacity = capacity,
    .length = length,
    .val = s
  };

  return str;
}

bool set_str(string* str, char* new_string) {
  size_t len = strlen(new_string);

  if(len > str->capacity) {
    char* new_val = realloc(str->val, len + 1);

    if(new_val == NULL) {
      free(new_val);
      return false;
    }

    str->val = new_val;
    str->capacity = len;
  }

  strncpy(str->val, new_string, len);
  str->val[len] = '\0';
  str->length = len;

  return true;
}

bool pop_char(string* str) {
  if(str->length <= 0) return false;
  str->val[str->length - 1] = '\0';
  str->length--;
  return true;
}

bool remove_char(string* str, int idx) {
  assert(idx < str->length && idx >= 0);
  if(str->length <= 0 || idx < 0 || idx >= str->length) return false;
  for(int i = idx; i < str->length; i++) str->val[i] = str->val[i + 1];
  str->val[str->length - 1] = '\0';
  str->length--;
  return true;
}

bool append_char(string* str, char c) {
  if((str->length + 1) > str->capacity) {
    char* new_val = realloc(str->val, str->length + 2);

    if(new_val == NULL) {
      free(new_val);
      return false;
    }

    str->val = new_val;
    str->capacity = str->length + 1;
  }

  str->length++;
  str->val[str->length - 1] = c;
  str->val[str->length] = '\0';

  return true;
}

bool insert_char(string* str, char c, int idx) {
  assert(idx <= str->length && idx >= 0);
  if((str->length + 1) > str->capacity) {
    char* new_val = realloc(str->val, str->length + 2);

    if(new_val == NULL) {
      free(new_val);
      return false;
    }

    str->val = new_val;
    str->capacity = str->length + 1;
  }

  str->length++;
  str->val[str->length] = '\0';
  
  for(int i = str->length; i > idx; i--) str->val[i] = str->val[i - 1];
  str->val[idx] = c;

  return true;
}

bool str_cat_str(string* dest, string* src) {
  size_t new_len = dest->length + src->length;
  size_t new_cap = dest->capacity < src->capacity && dest->capacity > new_len ? new_len : dest->capacity;
  
  char* new_val = realloc(dest->val, new_cap + 1);

  if(new_val == NULL) {
    free(new_val);
    return false;
  }

  strncat(new_val, src->val, src->length);
  new_val[new_len] = '\0';

  dest->val = new_val;
  dest->length = new_len;
  dest->capacity = new_cap;

  return true;
}

bool str_cat_charptr(string* dest, const char* str) {
  size_t sl = strlen(str);
  size_t new_len = dest->length + sl;
  size_t new_cap = (dest->capacity < new_len) ? new_len : dest->capacity;
  
  char* new_val = realloc(dest->val, new_cap + 1);

  if(new_val == NULL) {
    free(new_val);
    return false;
  }

  strncat(new_val, str, sl);
  new_val[new_len] = '\0';

  dest->capacity = new_cap;
  dest->length = new_len;
  dest->val = new_val;

  return true;
}

void print_string(string str) {
  printf("{ val: \"%s\", length: %lu, capacity: %lu }\n", str.val, str.length, str.capacity);
}