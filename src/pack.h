#ifndef PACK_H
#define PACK_H

#include <stdlib.h>
#include <stdint.h>

/* round/align x to 0x1000 (0x5475 -> 0x6000) */
#define ROUND(x) (((((uintptr_t)(x))-1)|0xFFF) + 1)

#define PLACEHOLDER_ENTRY 0x44444444
#define PLACEHOLDER_TEXT_OFF 0x33333333
#define PLACEHOLDER_TEXT_SIZE 0x22222222

uint32_t *find_32_placeholder(uint32_t placeholder, void *bootloader_start, size_t bl_len);
int check_offset(uintptr_t offset, uintptr_t elem_count, uintptr_t elem_size, uintptr_t size);

#define ASSERT_OFFSET(offset, elem_count, elem_size, size, ret)     \
    if (check_offset(                                               \
        (intptr_t)(offset), (intptr_t)(elem_count),                 \
        (intptr_t)(elem_size), (intptr_t)(size)) < 0) return (ret)

#endif