#ifndef __UTILS_H__
#define __UTILS_H__
#define __private static

#include <stdlib.h>

char *str_dup(const char *src, size_t len);
char *center_string(char *buff, int f_width);
void free_str_list(char **list, size_t len);

#define __cleanup(FNC)      __attribute__((__cleanup__(FNC)))
#endif
