TARGET = ./woody_woodpacker
C_SRCS = src/main.c
S_SRCS = src/bootloader/x86_64.s
CFLAGS = -Wall -Wextra -Werror -g3
OBJS   = $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)
CC     = gcc -m64
NASM   = nasm -f elf64

all: ${TARGET} .PH0NY

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.s
	${NASM} -o $@ -f elf64 $<

re: clean fclean .PH0NY all

clean: .PH0NY
	rm -f ${OBJS}

fclean: clean .PH0NY
	rm -f ${TARGET}

.PH0NY: