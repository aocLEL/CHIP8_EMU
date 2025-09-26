#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <utility/utils.h>

char *str_dup(const char *src, size_t len) {
  if(!len) len = strlen(src);
  char *buff = malloc(sizeof(char) * (len + 1));
  memcpy(buff, src, len);
  buff[len] = 0;
  return buff;
}



char *center_string(char *buff, int f_width) {
  char *str = str_dup(buff, 0);
  size_t len = strlen(str);
  sprintf(buff, "%*s%*s", (int)(((f_width+len)/2) & INT_MAX), str, (int)(((f_width-len)/2) & INT_MAX), "");
  free(str);
  return buff;
}

void free_str_list(char **list, size_t len) {
  for(size_t i = 0; i < len; i++)
    free(list[i]);
  free(list);
}
