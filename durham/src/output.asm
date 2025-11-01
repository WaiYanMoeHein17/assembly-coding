section .data
    digit db '0', 10
    array times 1000 dq 0

section .text
    global main
    extern putchar

main:
    push rbp
    mov rbp, rsp
    sub rsp, 1024

    mov rcx, 48
    call putchar
    mov rcx, 49
    call putchar
    mov rcx, 50
    call putchar
    mov rcx, 51
    call putchar
    mov rcx, 52
    call putchar
    mov rcx, 53
    call putchar
    mov rcx, 54
    call putchar
    mov rcx, 55
    call putchar
    mov rcx, 56
    call putchar
    mov rcx, 57
    call putchar
    mov rcx, 10
    call putchar

    mov rcx, 50
    call putchar
    mov rcx, 10
    call putchar

    mov rcx, 52
    call putchar
    mov rcx, 10
    call putchar

    mov rcx, 57
    call putchar
    mov rcx, 10
    call putchar

    mov rcx, 50
    call putchar
    mov rcx, 53
    call putchar
    mov rcx, 10
    call putchar

    ; Variable 'v' size=3 at [rbp-8]
    mov qword [rbp-8], 3
    mov qword [rbp-16], 1
    mov qword [rbp-24], 1
    mov qword [rbp-32], 1

    ; Print vector 'v'
    mov r10, [rbp-8]  ; load size
    lea r11, [rbp-16]  ; array start
    mov rcx, 40
    call putchar
    xor r12, r12
.print_loop_v:
    cmp r12, r10
    jge .print_done_v
    mov rax, [r11 + r12*8]
    add rax, 48
    mov rcx, rax
    call putchar
    inc r12
    cmp r12, r10
    jge .print_done_v
    mov rcx, 44
    call putchar
    jmp .print_loop_v
.print_done_v:
    mov rcx, 41
    call putchar
    mov rcx, 10
    call putchar

    xor rax, rax
    add rsp, 1024
    pop rbp
    ret
