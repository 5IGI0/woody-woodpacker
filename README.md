# woody_woodpacker

**woody_woodpacker** is an educational project from the 42 school.
The goal is to learn about binary formats (ELF) and basic packing techniques by creating a program that reads an executable, injects a small loader (stub), and hides (encrypts/obfuscates) the original code until runtime.

> Pedagogical objective: understand ELF headers and program headers, how to allocate space inside a binary, how to inject a small loader that decrypts and transfers execution back to the original code, and the constraints that arise when modifying an executable.

## Ethical & security notice

This project explores techniques that can be misused (obfuscation, evasion of defensive tools). woody_woodpacker is intended strictly for educational and research use. Do not use these techniques to compromise systems, bypass legitimate protections, or distribute malicious code. Always comply with applicable laws and your organization’s policies.

## Features

- Read 32/64-bit (x86) ELF binaries.
- Inject a loader (stub) into the target binary.
- Apply a simple, pedagogical encryption to the original code segment.
- Modify ELF headers/segments so the loader runs at program start.
- Loader decrypts the original code in memory and transfers execution back.

## Example

```
~$ objdump -d a.out
[...]
0000000000001139 <main>:
    1139:       55                      push   %rbp
    113a:       48 89 e5                mov    %rsp,%rbp
    113d:       48 8d 05 c0 0e 00 00    lea    0xec0(%rip),%rax        # 2004 <_IO_stdin_used+0x4>
    1144:       48 89 c7                mov    %rax,%rdi
    1147:       e8 e4 fe ff ff          call   1030 <puts@plt>
    114c:       b8 00 00 00 00          mov    $0x0,%eax
    1151:       5d                      pop    %rbp
    1152:       c3                      ret
[...]
~$ ./woody_woodpacker a.out
key: f8732714a0a018f6066c10b131c0daa1
~$ objdump -d woody
[...]
0000000000001139 <main>:
    1139:       39 58 38                cmp    %ebx,0x38(%rax)
    113c:       d4                      (bad)
    113d:       88 57 a4                mov    %dl,-0x5c(%rdi)
    1140:       38 7d 27                cmp    %bh,0x27(%rbp)
    1143:       14 e8                   adc    $0xe8,%al
    1145:       29 df                   sub    %ebx,%edi
    1147:       1e                      (bad)
    1148:       e2 92                   loop   10dc <register_tm_clones+0x2c>
    114a:       ef                      out    %eax,(%dx)
    114b:       4e 89 c0                rex.WRX mov %r8,%rax
    114e:       da                      .byte 0xda
    114f:       a1                      .byte 0xa1
    1150:       f8                      clc
    1151:       2e                      cs
    1152:       e4                      .byte 0xe4
[...]
~$ ./a.out
Hello, World!
~$ ./woody
....WOODY....
Hello, World!
```