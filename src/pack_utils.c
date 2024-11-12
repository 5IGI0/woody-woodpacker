#include <stdint.h>
#include <unistd.h>

#include "ft.h"

uint32_t *find_32_placeholder(uint32_t placeholder, void *bootloader_start, size_t bl_len) {
    return (uint32_t *)ft_memmem(bootloader_start, bl_len, &placeholder, sizeof(uint32_t));
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