section .text
    global _start
_start: 
    mov rax, 60
    mov rdi, 0
    syscall

section .data
    msg db 'Hello, World!', 0xA
    len equ $ - msg
