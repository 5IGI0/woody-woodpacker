#include <stdlib.h>

int ft_memcmp(const void *a, const void *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int ret = ((char *)a)[i] - ((char *)b)[i];
        if (ret != 0)
            return ret;
    }

    return 0;
}

void const *ft_memmem(const void *haystack, size_t heystacklen, const void *needle, size_t needlelen) {
    if (needlelen > heystacklen) {
        return NULL;
    }

    for (size_t i = 0; i < (heystacklen - needlelen + 1); i++) {
        if (ft_memcmp(haystack+i, needle, needlelen) == 0) {
            return haystack+i;
        }
    }

    return NULL;
}

void *ft_memcpy(void *dst, const void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char *)dst)[i] = ((const char *)src)[i];
    }

    return dst;
}

void *ft_memset(void *dst, int c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((unsigned char *)dst)[i] = c;
    }

    return dst;
}