#ifndef BOOTLOADER_H
#define BOOTLOADER_H
#include <stdint.h>

#define PLACEHOLDER_ENTRY 0x44444444
#define PLACEHOLDER_TEXT_OFF 0x33333333
#define PLACEHOLDER_TEXT_SIZE 0x22222222

extern unsigned char bootloader_64[];
extern uint32_t bootloader_64_len;
extern unsigned char bootloader_32[];
extern uint32_t bootloader_32_len;

#endif
