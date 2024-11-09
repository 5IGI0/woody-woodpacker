#!/bin/sh

nasm -f bin -o bootloader bootloader.asm

cat > bootloader.h << EOF 
#ifndef BOOTLOADER_H
#define BOOTLOADER_H
EOF

xxd --include bootloader >> bootloader.h

cat >> bootloader.h << EOF 
#endif
EOF