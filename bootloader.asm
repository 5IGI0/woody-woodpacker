bits 64

global _start

SECTION .text
    _start:
        ; save initial registers' values
        push rsi
        push rdx
        push rax

        ; display "....WOODY....\n"
        mov rdi, 1
        mov rdx, woody_len
        lea rsi, [rel woody_str]
        ;mov rsi, rsp
        mov rax, 1
        syscall

        ; TODO: decrypt .text

        ; restore registers
        pop rax
        pop rdx
        pop rsi

        ; jmp to the entry point
        jmp 0x44444448

        woody_str:
        db "...WOODY...", 0x0A
        woody_len equ $ - woody_str