bits 64

global _start

SECTION .text
    _start:
        mov rsi, 0x0a2e2e2e2e59 ; Y....\n
        push rsi
        mov rsi, 0x444f4f572e2e2e2e ; ...WOOD
        push rsi
        mov rdi, 1
        mov rdx, 14
        mov rsi, rsp
        mov rax, 1
        syscall
        mov rax, 60
        syscall