bits 64

global bootloader_64
global bootloader_64_len

SECTION .data
    bootloader_64:
        ; save initial registers' values
        push rsi
        push rdx
        push rax
        push rbx

        ; display "....WOODY....\n"
        mov rdi, 1
        mov rdx, woody_len
        lea rsi, [rel woody_str]
        ;mov rsi, rsp
        mov rax, 1
        syscall

        ; RAX = .text start
        ; RDX = .text size
        ; RSI = offset
        ; RBX = temp memory
        lea rax, [rel bootloader_64]
        sub rax, 0x33333333
        mov rdx, 0x22222222
        xor rsi, rsi

        decrypt_loop:
            mov bl, byte [rax+rsi]
            xor bl, 0x58
            mov byte [rax+rsi], bl
            inc rsi
            cmp rsi, rdx
        jb decrypt_loop

        ; restore registers
        pop rbx
        pop rax
        pop rdx
        pop rsi

        ; jmp to the entry point
        jmp 0x44444448 ; + 4 because nasm sub 4

        woody_str:
        db "....WOODY....", 0x0A
        woody_len equ $ - woody_str
        bootloader_64_len:
        db DWORD $ - bootloader_64

SECTION .note.GNU-stack