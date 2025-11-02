section .data
    loop_var dd 0       ; Counter for number of inputs processed
    div_val dd 3        ; Constant 3 (divisor)
    prompt db "Enter Number: ", 0 
    prompt_len equ $ - prompt
    newline db 10       ; newline character
    
    ; Magic constant for division by 3 (unsigned 32-bit)
    ; 0xAAAAAAAB * n >> 33 gives n/3
    magic_mult dq 0x0000000055555556  ; For signed: (n * 0x55555556) >> 32

section .bss
    input_buffer resb 16     ; Buffer for input
    num_str resb 12          ; Buffer for number output
    input resd 1             ; converted integer value
    quotient resd 1          ; result
    remainder resd 1         ; remainder

section .text
    global _start

; Convert ASCII string to integer
atoi:
    xor rax, rax            ; result = 0
    xor rcx, rcx            ; sign = positive
    mov rsi, input_buffer   ; pointer to string
    
    ; Check for negative sign
    cmp byte [rsi], '-'
    jne .parse_loop
    inc rsi                 ; skip '-'
    mov rcx, 1              ; mark as negative
    
.parse_loop:
    movzx rdx, byte [rsi]   ; get character
    cmp rdx, 10             ; check for newline
    je .done
    cmp rdx, 0              ; check for null
    je .done
    cmp rdx, '0'
    jl .done
    cmp rdx, '9'
    jg .done
    
    sub rdx, '0'            ; convert ASCII to digit
    imul rax, 10            ; result *= 10
    add rax, rdx            ; result += digit
    inc rsi
    jmp .parse_loop
    
.done:
    test rcx, rcx           ; check if negative
    jz .positive
    neg rax                 ; negate result
.positive:
    ret

; Convert integer to ASCII string
itoa:
    ; rax = number to convert
    ; rdi = buffer
    mov rbx, 10
    mov rcx, 0              ; digit count
    
    ; Handle negative numbers
    test rax, rax
    jns .positive_num
    neg rax
    mov byte [rdi], '-'
    inc rdi
    
.positive_num:
    ; Convert digits (in reverse)
.divide_loop:
    xor rdx, rdx
    div rbx                 ; rax = rax/10, rdx = remainder
    add rdx, '0'            ; convert to ASCII
    push rdx
    inc rcx
    test rax, rax
    jnz .divide_loop
    
    ; Pop digits in correct order
.pop_loop:
    pop rax
    mov [rdi], al
    inc rdi
    loop .pop_loop
    
    mov byte [rdi], 10      ; newline
    inc rdi
    mov byte [rdi], 0       ; null terminator
    ret

_start:
    xor r12, r12            ; loop counter

input_loop:
    ; Check if we've processed 3 inputs
    cmp r12, 3
    jge exit_program

    ; Print prompt
    mov rax, 1              ; sys_write
    mov rdi, 1              ; stdout
    mov rsi, prompt
    mov rdx, prompt_len
    syscall

    ; Read input
    mov rax, 0              ; sys_read
    mov rdi, 0              ; stdin
    mov rsi, input_buffer
    mov rdx, 16
    syscall

    ; Convert input to integer
    call atoi
    mov [input], eax

    ; OPTIMIZED DIVISION BY 3 using magic constant
    ; For signed integers: quotient = (n * 0x55555556) >> 32
    ; This is much faster than repeated subtraction!
    
    mov eax, [input]        ; load number
    mov r8d, eax            ; save original for remainder calc
    
    ; Multiply by magic constant
    imul rax, qword [magic_mult]
    sar rax, 32             ; shift right 32 bits (signed)
    
    ; Handle negative numbers: add (n >> 31) to adjust
    mov ecx, r8d
    sar ecx, 31             ; get sign bit
    add eax, ecx            ; adjust for negative
    
    mov [quotient], eax     ; store quotient
    
    ; Calculate remainder: remainder = n - (quotient * 3)
    imul eax, 3             ; quotient * 3
    sub r8d, eax            ; n - (quotient * 3)
    mov [remainder], r8d
    
    ; Print quotient
    mov rax, [quotient]
    lea rdi, [num_str]
    call itoa
    
    mov rax, 1              ; sys_write
    mov rdi, 1              ; stdout
    mov rsi, num_str
    mov rdx, 12
    syscall

    ; Increment loop counter
    inc r12
    jmp input_loop

exit_program:
    mov rax, 60             ; sys_exit
    xor rdi, rdi            ; exit code 0
    syscall