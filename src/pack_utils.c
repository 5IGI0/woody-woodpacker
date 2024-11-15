#include <stdint.h>
#include <unistd.h>

#include "ft.h"

uint32_t *find_32_placeholder(uint32_t placeholder, void *bootloader_start, size_t bl_len) {
    return (uint32_t *)ft_memmem(bootloader_start, bl_len, &placeholder, sizeof(uint32_t));
}

char *find_128_placeholder(char *placeholder, void *bootloader_start, size_t bl_len) {
    return (char *)ft_memmem(bootloader_start, bl_len, placeholder, 16);
}

int check_offset(uintptr_t offset, uintptr_t elem_count, uintptr_t elem_size, uintptr_t size) {
    /* check elem size / elem_size*elem_count overflow */
    if (
        elem_size > INTPTR_MAX ||
        (elem_size > 0 && (INTPTR_MAX / elem_size) < elem_count)
    ) {
        write(2, "unexpected end of file.\n", 24);
        return -1;
    }

    /* check invalid offset */
    if (offset > INTPTR_MAX || size > INTPTR_MAX || (offset >= size)) {
        write(2, "invalid offset\n", 15);
        return -1;
    }

    /* check if there are enough space */
    if (elem_size*elem_count > (size - offset)) {
        write(2, "unexpected end of file.\n", 24);
        return -1;
    }

    return 0;
}

void bin2hex(char *out, unsigned char const *in, size_t inlen) {
  for (size_t i = 0; i < inlen; i++) {
    out[i*2] = "0123456789abcdef"[in[i]&0xF];
    out[i*2+1] = "0123456789abcdef"[in[i]>>4];
  }
}

void print_key(unsigned char *key)
{
    write(1 ,"key: ", 6);
    char str_key[32];
    bin2hex(str_key, key, 16);
    write(1, str_key, 32);
    write(1, "\n", 1);
}