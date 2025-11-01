    
section .text
    global _start
_start: 
    mov rax, 1          ; sys_write
    mov rdi, 1          ; sys_write
    mov rsi, print      ; buffer to write
    mov rdx, len        ; number of bytes
    syscall

    mov rax, 60         ; sys_exit
    xor rdi, rdi        ; status 0
    syscall

section .data
    print db 'Output', 0xa
    len equ $ - print