bits 32

global bootloader_32
global bootloader_32_len

SECTION .data
    bootloader_32:
        pushad
        
        ; call itself to know rip and get the bootloader's addr
        call dumb_func
        dumb_func:
        pop edi
        sub edi, dumb_func

        ; display "....WOODY...\n"
        mov eax, 0x4
        mov ebx, 0x1
        lea ecx, [edi + woody_str]
        mov edx, 0xE ; woody_len
        int 0x80

        ; eax = .text start
        ; edx = .text size
        ; esi = offset
        ; ebx = temp memory
        mov eax, edi
        sub eax, 0x33333333
        mov edx, 0x22222222
        xor esi, esi

        decrypt_loop:
            mov bl, byte [eax+esi]
            xor bl, 0x58
            mov byte [eax+esi], bl
            inc esi
            cmp esi, edx
        jb decrypt_loop
        
        popad

        jmp 0x44444448
    
    woody_str:
        db "....WOODY....", 0x0A
    bootloader_32_len:
        db DWORD $ - bootloader_32

SECTION .note.GNU-stack