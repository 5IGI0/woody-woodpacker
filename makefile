TARGET = ./woody_woodpacker
C_SRCS = src/main.c src/ft.c src/pack_utils.c
S_SRCS = src/bootloader/x86_64.64.s src/bootloader/x86.32.s
CFLAGS = -Wall -Wextra -Werror -O0 -g3
OBJS   = $(C_SRCS:.c=.o) $(S_SRCS:.s=.o) src/pack.64.o src/pack.32.o
CC     = gcc -m64
NASM   = nasm -f elf64

all: ${TARGET} .PH0NY

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.64.o: %.64.s
	${NASM} -o $@.tmp -f elf64 $<
	objcopy --remove-relocations .data -O elf64-x86-64 $@.tmp $@
	rm $@.tmp

%.32.o: %.32.s
	${NASM} -o $@.tmp -f elf32 $<
	objcopy --remove-relocations .data -O elf64-x86-64 $@.tmp $@
	rm $@.tmp

%.64.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ -DTARGET_64=1

%.32.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ -DTARGET_32=1

re: clean fclean .PH0NY all

clean: .PH0NY
	rm -f ${OBJS}

fclean: clean .PH0NY
	rm -f ${TARGET} woody

.PH0NY: