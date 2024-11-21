#ifndef FT_H
#define FT_H

#include <stdlib.h>

int ft_memcmp(const void *a, const void *b, size_t n);
void const *ft_memmem(const void *haystack, size_t heystacklen, const void *needle, size_t needlelen);
void *ft_memcpy(void *dst, const void *src, size_t n);
void *ft_memset(void *dst, int c, size_t n);
size_t ft_strlen(char const *str);

#endif