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
        push r9
        push r13

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
        ; R13 = temp_key
        ;  R9 = key addr
        lea rax, [rel bootloader_64]
        sub rax, 0x33333333
        mov rdx, 0x22222222
        xor rsi, rsi

        ; 14%16 = 14 | 14 (0b1110) & 15 (0b1111) = 14 (0b1110) 
        ; 15%16 = 15 | 15 (0b1111) & 15 (0b1111) = 15 (0b1111) 
        ; 16%16 = 0  | 16 (0b10000) & 15 (0b1111) = 0 (0b0000) 
        lea r9, [rel xor_key]

        decrypt_loop:
            mov r13, rsi                ; tmp = i
            and r13, 0xF                ; tmp &= 0xF (tmp %= 16) 0b1111
            mov r13b, byte [r9 + r13]   ; tmp = key[tmp]
            mov bl, byte [rax+rsi]      ; text_tmp = .text[i]
            xor bl, r13b                ; text_tmp ^= tmp
            mov byte [rax+rsi], bl      ; .text[i] = text_tmp
            inc rsi                     ; i++
            cmp rsi, rdx                ; if i < text_size; jump to decrypt_loop
        jb decrypt_loop

        ; restore registers
        pop r13
        pop r9
        pop rbx
        pop rax
        pop rdx
        pop rsi

        ; jmp to the entry point
        jmp 0x44444448 ; + 4 because nasm sub 4

        woody_str:
        db "....WOODY....", 0x0A
        woody_len equ $ - woody_str
        xor_key:
            db "19-45-kouks14-88"
        bootloader_64_len:
        db DWORD $ - bootloader_64

SECTION .note.GNU-stack