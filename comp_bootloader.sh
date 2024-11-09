#!/bin/sh

nasm -f elf64 -o .bootloader.obj bootloader.asm
objcopy .bootloader.obj -O binary bootloader

cat > bootloader.h << EOF 
#ifndef BOOTLOADER_H
#define BOOTLOADER_H
EOF

xxd --include bootloader >> bootloader.h

cat >> bootloader.h << EOF 
#endif
EOF

rm .bootloader.obj bootloader