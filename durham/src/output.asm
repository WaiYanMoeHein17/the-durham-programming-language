section .data
    digit db '0', 10
    array times 1000 dq 0

section .bss
    temp_buffer resb 32
    heap_space resb 8192
    heap_ptr resq 1

section .text
    global main
    extern putchar

main:
    push rbp
    mov rbp, rsp
    sub rsp, 1024

    ; Initialize heap pointer
    lea rax, [rel heap_space]
    mov [rel heap_ptr], rax

    mov [rbp-8], rax
    mov rax, [rbp-8]
    ; Print value in rax
    test rax, rax
    jnz .not_zero_0
    mov rcx, '0'
    call putchar
    jmp .done_print_0
.not_zero_0:
    mov rbx, 10
    xor r12, r12
    lea r13, [rel temp_buffer]
.digit_loop_0:
    xor rdx, rdx
    div rbx
    add dl, '0'
    mov [r13 + r12], dl
    inc r12
    test rax, rax
    jnz .digit_loop_0
.print_loop_0:
    dec r12
    movzx rcx, byte [r13 + r12]
    call putchar
    test r12, r12
    jnz .print_loop_0
.done_print_0:
    mov rcx, 10
    call putchar
    mov rax, 5
    mov [rbp-16], rax
    mov rax, [rbp-16]
    ; Print value in rax
    test rax, rax
    jnz .not_zero_1
    mov rcx, '0'
    call putchar
    jmp .done_print_1
.not_zero_1:
    mov rbx, 10
    xor r12, r12
    lea r13, [rel temp_buffer]
.digit_loop_1:
    xor rdx, rdx
    div rbx
    add dl, '0'
    mov [r13 + r12], dl
    inc r12
    test rax, rax
    jnz .digit_loop_1
.print_loop_1:
    dec r12
    movzx rcx, byte [r13 + r12]
    call putchar
    test r12, r12
    jnz .print_loop_1
.done_print_1:
    mov rcx, 10
    call putchar

    xor eax, eax
    add rsp, 1024
    pop rbp
    ret
