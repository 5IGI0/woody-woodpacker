bits 64

global _start

SECTION .text
    _start:
        ; save initial registers' values
        push rsi
        push rdx
        push rax

        ; display "....WOODY....\n"
        mov rsi, 0x0a2e2e2e2e59 ; Y....\n
        push rsi
        mov rsi, 0x444f4f572e2e2e2e ; ...WOOD
        push rsi
        mov rdi, 1
        mov rdx, 14
        mov rsi, rsp
        mov rax, 1
        syscall

        ; TODO: decrypt .text

        ; restore registers
        sub rsp, 16
        pop rax
        pop rdx
        pop rsi

        ; jmp to the entry point
        jmp 0x44444448