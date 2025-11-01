section .data
    loop_var dd 0       ; Counter for number of inputs processed
    a dd 0              ; Accumulator/remainder
    count dd 0          ; Quotient (result)
    one dd 1            ; Constant 1
    div_val dd 3        ; Constant 3 (divisor)
    prompt db "Enter Number: ", 0 
    prompt_len equ $ - prompt
    newline db 10       ; newline character

section .bss
    input_buffer resb 16     ; Buffer for input
    input resd 1             ; convert integer value

section .text
    global _start

_start:
    ; Reset variables (LDA loop, STO a, STO count)
    xor rax, rax                    ; sets eax to 0
    mov [a], rax                    ; moves eax which is 0 into a
    mov [count], rax                ; moves eax which is 0 into count

start:
    ; SUB div - check if we've processed 3 inputs
    mov rax, [loop_var]             ; mov into eax, the loop_var
    sub rax, [div_val]              ; subtraction eax - (div_val = 3), if 0 then jz aend
    jz aend             ; BRZ aend - if loop == 3, go to end

    ; IN - get input (simplified - would need actual input handling)
    ; For demonstration, assuming input is in a register or memory
    ; In real implementation, you'd call input functions here

    ;prompt
    mov rax, 1                      ; sys_write
    mov rdi, 1                      ; stdout
    mov rsi, prompt                 ; buffer for input
    mov rdx, prompt_len             ; length
    syscall

    ;input 
    mov rax, 0                      ; sys_read
    mov rdi, 0                      ; stdin
    mov rsi, input_buffer           ; 
    mov rdx, 16 
    syscall

    
    ; SUB div, ADD a
    mov rax, input_buffer    ; Load input
    sub rax, div_val         ; SUB div
    add eax, a               ; ADD a
    jns adc                  ; BRP adc - if positive, jump to adc

    ; ADD div, STO a
    add eax, [div_val]  ; Add 3 back to get remainder
    mov [a], eax        ; Store remainder

divid:
    ; LDA a, SUB div
    mov eax, [a]
    sub eax, [div_val]
    jns adc             ; BRP adc - if positive, can subtract 3 again

    ; ADD div, STO a - get remainder
    add eax, [div_val]
    mov [a], eax

    ; LDA loop, ADD one, STO loop
    mov eax, [loop_var]
    add eax, [one]
    mov [loop_var], eax

    ; BR start
    jmp start

adc:
    ; STO a - store the result after subtraction
    mov [a], eax

    ; LDA count, ADD one, STO count
    mov eax, [count]
    add eax, [one]
    mov [count], eax

    ; BR divid
    jmp divid

aend:
    ; STO loop (store zero to loop)
    mov [loop_var], eax

    ; LDA count, OUT
    mov eax, [count]
    ; Output result (would need actual output handling)
    ; For demonstration, result is in EAX

    ; Exit program (HLT equivalent)
    mov eax, 1          ; sys_exit
    xor ebx, ebx        ; exit code 0
    int 0x80