#ifndef BOOTLOADER_H
#define BOOTLOADER_H
unsigned char bootloader[] = {
  0x56, 0x52, 0x50, 0xbf, 0x01, 0x00, 0x00, 0x00, 0xba, 0x0c, 0x00, 0x00,
  0x00, 0x48, 0x8d, 0x35, 0x0f, 0x00, 0x00, 0x00, 0xb8, 0x01, 0x00, 0x00,
  0x00, 0x0f, 0x05, 0x58, 0x5a, 0x5e, 0xe9, 0x44, 0x44, 0x44, 0x44, 0x2e,
  0x2e, 0x2e, 0x57, 0x4f, 0x4f, 0x44, 0x59, 0x2e, 0x2e, 0x2e, 0x0a
};
unsigned int bootloader_len = 47;
#endif
