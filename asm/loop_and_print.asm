section .text
    global _start
_start:
    mov r12, 10

.while_loop_start: 
    cmp r12, 0
    jle .while_loop_end

    mov rax, 1
    mov rdi, 1
    mov rsi, msg
    mov rdx, len
    syscall
    
    dec r12                 ; decrementing by 1
    jmp. .while_loop_start

.while_loop_end: 
    mov rax, 60
    xor rdi, rdi
    syscall

section .data
msg db 'Hello World!',0xa
len equ $ - msg