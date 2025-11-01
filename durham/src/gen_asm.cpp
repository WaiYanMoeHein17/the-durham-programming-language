#include "main.h"

// Helper function to convert base-17 string to decimal integer
int base17_to_decimal(const std::string& base17_str) {
    int decimal_value = 0;
    for (char c : base17_str) {
        decimal_value *= 17;
        if (c >= '0' && c <= '9') {
            decimal_value += (c - '0');
        } else if (c >= 'A' && c <= 'G') {
            decimal_value += (c - 'A' + 10);
        }
    }
    return decimal_value;
}

// Helper function to generate unique labels
static int label_counter = 0;
std::string generate_label(const std::string& prefix) {
    return prefix + std::to_string(label_counter++);
}

// Helper function to evaluate a condition and generate comparison code at runtime
std::string generate_condition(const std::vector<Token>& condition_tokens, 
                               const std::map<std::string, int>& scalars,
                               std::stringstream& assembly_code) {
    // Handle compound conditions with OR: cond1 or cond2
    // For OR: if either condition is true, don't jump (continue)
    if (condition_tokens.size() == 7 && condition_tokens[3].type == TokenType::_or) {
        std::string or_success = generate_label("or_success_");
        
        // Evaluate first condition (tokens 0-2)
        if (condition_tokens[0].type == TokenType::int_lit) {
            int left_val = base17_to_decimal(condition_tokens[0].value.value());
            assembly_code << "    mov rax, " << left_val << "\n";
        } else if (condition_tokens[0].type == TokenType::identifier) {
            std::string var_name = condition_tokens[0].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rax, [rbp" << offset << "]\n";
            }
        }
        
        if (condition_tokens[2].type == TokenType::int_lit) {
            int right_val = base17_to_decimal(condition_tokens[2].value.value());
            assembly_code << "    mov rbx, " << right_val << "\n";
        } else if (condition_tokens[2].type == TokenType::identifier) {
            std::string var_name = condition_tokens[2].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rbx, [rbp" << offset << "]\n";
            }
        }
        
        assembly_code << "    cmp rax, rbx\n";
        
        // If first condition is true, jump to success (don't skip body)
        if (condition_tokens[1].type == TokenType::_equals) {
            assembly_code << "    je " << or_success << "\n";
        } else if (condition_tokens[1].type == TokenType::_greater) {
            assembly_code << "    jg " << or_success << "\n";
        } else if (condition_tokens[1].type == TokenType::_lesser) {
            assembly_code << "    jl " << or_success << "\n";
        }
        
        // Evaluate second condition (tokens 4-6)
        if (condition_tokens[4].type == TokenType::int_lit) {
            int left_val = base17_to_decimal(condition_tokens[4].value.value());
            assembly_code << "    mov rax, " << left_val << "\n";
        } else if (condition_tokens[4].type == TokenType::identifier) {
            std::string var_name = condition_tokens[4].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rax, [rbp" << offset << "]\n";
            }
        }
        
        if (condition_tokens[6].type == TokenType::int_lit) {
            int right_val = base17_to_decimal(condition_tokens[6].value.value());
            assembly_code << "    mov rbx, " << right_val << "\n";
        } else if (condition_tokens[6].type == TokenType::identifier) {
            std::string var_name = condition_tokens[6].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rbx, [rbp" << offset << "]\n";
            }
        }
        
        assembly_code << "    cmp rax, rbx\n";
        
        // Return jump instruction for second condition (if false, skip body)
        std::string jump_instr;
        if (condition_tokens[5].type == TokenType::_equals) {
            jump_instr = "jne";
        } else if (condition_tokens[5].type == TokenType::_greater) {
            jump_instr = "jle";
        } else if (condition_tokens[5].type == TokenType::_lesser) {
            jump_instr = "jge";
        }
        
        assembly_code << or_success << ":\n";
        return jump_instr;
    }
    // Handle simple comparisons: var1 > var2, var1 < var2, etc.
    else if (condition_tokens.size() == 3) {
        // Load left operand into rax
        if (condition_tokens[0].type == TokenType::int_lit) {
            int left_val = base17_to_decimal(condition_tokens[0].value.value());
            assembly_code << "    mov rax, " << left_val << "\n";
        } else if (condition_tokens[0].type == TokenType::identifier) {
            std::string var_name = condition_tokens[0].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rax, [rbp" << offset << "]\n";
            } else {
                assembly_code << "    mov rax, 0  ; undefined variable\n";
            }
        }
        
        // Load right operand into rbx
        if (condition_tokens[2].type == TokenType::int_lit) {
            int right_val = base17_to_decimal(condition_tokens[2].value.value());
            assembly_code << "    mov rbx, " << right_val << "\n";
        } else if (condition_tokens[2].type == TokenType::identifier) {
            std::string var_name = condition_tokens[2].value.value();
            if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars.at(var_name);
                assembly_code << "    mov rbx, [rbp" << offset << "]\n";
            } else {
                assembly_code << "    mov rbx, 0  ; undefined variable\n";
            }
        }
        
        assembly_code << "    cmp rax, rbx\n";
        
        // Return the jump instruction based on operator
        if (condition_tokens[1].type == TokenType::_greater) {
            return "jle";  // Jump if less or equal (opposite of >)
        } else if (condition_tokens[1].type == TokenType::_lesser) {
            return "jge";  // Jump if greater or equal (opposite of <)
        } else if (condition_tokens[1].type == TokenType::_equals) {
            return "jne";  // Jump if not equal
        } else if (condition_tokens[1].type == TokenType::_not_equals) {
            return "je";   // Jump if equal (opposite of !=)
        }
    }
    return "jmp";  // Default: unconditional jump
}

std::string generate_assembly(const std::vector<Token>& tokens) {
    std::stringstream assembly_code; 
    
    std::map<std::string, int> variables;  // For vectors (stores offset to size)
    std::map<std::string, int> scalars;    // For scalar variables (stores stack offset)
    int stack_offset = 0;

    assembly_code << "section .data\n";
    assembly_code << "    digit db '0', 10\n";
    assembly_code << "    array times 1000 dq 0\n\n";
    assembly_code << "section .bss\n";
    assembly_code << "    temp_buffer resb 32\n\n";
    assembly_code << "section .text\n";
    assembly_code << "    global main\n";
    assembly_code << "    extern putchar\n\n";
    assembly_code << "main:\n";
    assembly_code << "    push rbp\n";
    assembly_code << "    mov rbp, rsp\n";
    assembly_code << "    sub rsp, 1024\n\n";

    for (size_t i = 0; i < tokens.size(); i++) {
        // Handle vector assignment: name = (num, num, num);
        if (i + 2 < tokens.size() &&
            tokens[i].type == TokenType::identifier &&
            tokens[i + 1].type == TokenType::equals &&
            tokens[i + 2].type == TokenType::open_paren) {
            
            std::string var_name = tokens[i].value.value();
            i += 3; // Skip name, =, (
            
            std::vector<int> numbers;
            
            // Collect numbers
            while (i < tokens.size() && tokens[i].type != TokenType::close_paren) {
                if (tokens[i].type == TokenType::int_lit) {
                    numbers.push_back(base17_to_decimal(tokens[i].value.value()));
                }
                i++;
            }
            
            // Allocate stack space
            stack_offset -= 8;
            variables[var_name] = stack_offset;
            
            assembly_code << "    ; Variable '" << var_name << "' size=" << numbers.size() << " at [rbp" << stack_offset << "]\n";
            assembly_code << "    mov qword [rbp" << stack_offset << "], " << numbers.size() << "\n";
            
            for (size_t j = 0; j < numbers.size(); j++) {
                stack_offset -= 8;
                assembly_code << "    mov qword [rbp" << stack_offset << "], " << numbers[j] << "\n";
            }
            assembly_code << "\n";
        }
        // Handle scalar assignment with expression: name = var1 op var2 (CHECK THIS FIRST before simple assignment)
        else if (i + 4 < tokens.size() &&
                 tokens[i].type == TokenType::identifier &&
                 tokens[i + 1].type == TokenType::equals &&
                 (tokens[i + 3].type == TokenType::_durham ||
                  tokens[i + 3].type == TokenType::_newcastle ||
                  tokens[i + 3].type == TokenType::_york ||
                  tokens[i + 3].type == TokenType::_edinburgh)) {
            
            std::string var_name = tokens[i].value.value();
            
            // Allocate stack space if variable doesn't exist
            if (scalars.find(var_name) == scalars.end()) {
                stack_offset -= 8;
                scalars[var_name] = stack_offset;
            }
            
            // Load left operand
            if (tokens[i + 2].type == TokenType::int_lit) {
                int left_val = base17_to_decimal(tokens[i + 2].value.value());
                assembly_code << "    mov rax, " << left_val << "\n";
            } else if (tokens[i + 2].type == TokenType::identifier) {
                std::string left_var = tokens[i + 2].value.value();
                if (scalars.find(left_var) != scalars.end()) {
                    assembly_code << "    mov rax, [rbp" << scalars[left_var] << "]\n";
                }
            }
            
            // Load right operand
            if (tokens[i + 4].type == TokenType::int_lit) {
                int right_val = base17_to_decimal(tokens[i + 4].value.value());
                assembly_code << "    mov rbx, " << right_val << "\n";
            } else if (tokens[i + 4].type == TokenType::identifier) {
                std::string right_var = tokens[i + 4].value.value();
                if (scalars.find(right_var) != scalars.end()) {
                    assembly_code << "    mov rbx, [rbp" << scalars[right_var] << "]\n";
                }
            }
            
            // Perform operation
            if (tokens[i + 3].type == TokenType::_durham) {
                assembly_code << "    add rax, rbx\n";
            } else if (tokens[i + 3].type == TokenType::_newcastle) {
                assembly_code << "    sub rax, rbx\n";
            } else if (tokens[i + 3].type == TokenType::_york) {
                assembly_code << "    imul rax, rbx\n";
            } else if (tokens[i + 3].type == TokenType::_edinburgh) {
                assembly_code << "    xor rdx, rdx\n";
                assembly_code << "    idiv rbx\n";
            }
            
            int offset = scalars[var_name];
            assembly_code << "    mov [rbp" << offset << "], rax\n\n";
            
            i += 4; // Skip name, =, var1, op, var2
        }
        // Handle scalar assignment: name = number; (only if followed by semicolon or end)
        else if (i + 2 < tokens.size() &&
                 tokens[i].type == TokenType::identifier &&
                 tokens[i + 1].type == TokenType::equals &&
                 tokens[i + 2].type == TokenType::int_lit &&
                 (i + 3 >= tokens.size() || 
                  tokens[i + 3].type == TokenType::semi ||
                  tokens[i + 3].type == TokenType::close_paren ||
                  tokens[i + 3].type == TokenType::close_brace)) {
            
            std::string var_name = tokens[i].value.value();
            int value = base17_to_decimal(tokens[i + 2].value.value());
            
            // Allocate stack space if variable doesn't exist
            if (scalars.find(var_name) == scalars.end()) {
                stack_offset -= 8;
                scalars[var_name] = stack_offset;
            }
            
            int offset = scalars[var_name];
            assembly_code << "    ; Scalar variable '" << var_name << "' = " << value << "\n";
            assembly_code << "    mov qword [rbp" << offset << "], " << value << "\n\n";
            
            i += 2; // Skip name, =, number (semicolon handled by main loop)
        }
        // Handle while loop: while begin condition end front body back
        else if (tokens[i].type == TokenType::_while &&
                 i + 2 < tokens.size() &&
                 tokens[i + 1].type == TokenType::open_paren) {
            
            std::string loop_start = generate_label("while_start_");
            std::string loop_end = generate_label("while_end_");
            
            i += 2; // Skip 'while' and 'begin'
            
            // Collect condition tokens
            std::vector<Token> condition_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::close_paren) {
                condition_tokens.push_back(tokens[i]);
                i++;
            }
            i++; // Skip 'end'
            
            if (i < tokens.size() && tokens[i].type == TokenType::open_brace) {
                i++; // Skip 'front'
                
                assembly_code << loop_start << ":\n";
                
                // Generate condition check
                std::string jump_instr = generate_condition(condition_tokens, scalars, assembly_code);
                assembly_code << "    " << jump_instr << " " << loop_end << "\n\n";
                
                // Process loop body - collect body tokens and recursively process
                std::vector<Token> body_tokens;
                int brace_depth = 1;
                while (i < tokens.size() && brace_depth > 0) {
                    if (tokens[i].type == TokenType::open_brace) brace_depth++;
                    if (tokens[i].type == TokenType::close_brace) {
                        brace_depth--;
                        if (brace_depth == 0) break;
                    }
                    body_tokens.push_back(tokens[i]);
                    i++;
                }
                
                // Process body tokens inline
                for (size_t j = 0; j < body_tokens.size(); j++) {
                    // Handle scalar assignment in loop
                    if (j + 2 < body_tokens.size() &&
                        body_tokens[j].type == TokenType::identifier &&
                        body_tokens[j + 1].type == TokenType::equals &&
                        body_tokens[j + 2].type == TokenType::int_lit) {
                        
                        std::string var_name = body_tokens[j].value.value();
                        int value = base17_to_decimal(body_tokens[j + 2].value.value());
                        
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    mov qword [rbp" << offset << "], " << value << "\n";
                        }
                        j += 2;
                    }
                    // Handle expressions in loop
                    else if (j + 4 < body_tokens.size() &&
                             body_tokens[j].type == TokenType::identifier &&
                             body_tokens[j + 1].type == TokenType::equals &&
                             (body_tokens[j + 3].type == TokenType::_durham ||
                              body_tokens[j + 3].type == TokenType::_newcastle)) {
                        
                        std::string var_name = body_tokens[j].value.value();
                        
                        // Load left operand
                        if (body_tokens[j + 2].type == TokenType::identifier) {
                            std::string left_var = body_tokens[j + 2].value.value();
                            if (scalars.find(left_var) != scalars.end()) {
                                assembly_code << "    mov rax, [rbp" << scalars[left_var] << "]\n";
                            }
                        } else if (body_tokens[j + 2].type == TokenType::int_lit) {
                            int val = base17_to_decimal(body_tokens[j + 2].value.value());
                            assembly_code << "    mov rax, " << val << "\n";
                        }
                        
                        // Load right operand
                        if (body_tokens[j + 4].type == TokenType::identifier) {
                            std::string right_var = body_tokens[j + 4].value.value();
                            if (scalars.find(right_var) != scalars.end()) {
                                assembly_code << "    mov rbx, [rbp" << scalars[right_var] << "]\n";
                            }
                        } else if (body_tokens[j + 4].type == TokenType::int_lit) {
                            int val = base17_to_decimal(body_tokens[j + 4].value.value());
                            assembly_code << "    mov rbx, " << val << "\n";
                        }
                        
                        // Perform operation
                        if (body_tokens[j + 3].type == TokenType::_durham) {
                            assembly_code << "    add rax, rbx\n";
                        } else if (body_tokens[j + 3].type == TokenType::_newcastle) {
                            assembly_code << "    sub rax, rbx\n";
                        }
                        
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    mov [rbp" << offset << "], rax\n";
                        }
                        j += 4;
                    }
                    // Handle tlc() in loop
                    else if (j + 3 < body_tokens.size() &&
                             body_tokens[j].type == TokenType::_tlc &&
                             body_tokens[j + 1].type == TokenType::open_paren &&
                             body_tokens[j + 2].type == TokenType::identifier &&
                             body_tokens[j + 3].type == TokenType::close_paren) {
                        
                        std::string var_name = body_tokens[j + 2].value.value();
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    ; Print variable " << var_name << "\n";
                            assembly_code << "    mov rax, [rbp" << offset << "]\n";
                            
                            // Special case for zero
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .not_zero_" << label_counter << "\n";
                            assembly_code << "    mov rcx, '0'\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    jmp .done_print_" << label_counter << "\n";
                            assembly_code << ".not_zero_" << label_counter << ":\n";
                            
                            // Convert to string and print (store in buffer)
                            assembly_code << "    lea r13, [rel temp_buffer]\n";
                            assembly_code << "    mov rbx, 10\n";
                            assembly_code << "    xor r12, r12\n";
                            assembly_code << ".digit_loop_" << label_counter << ":\n";
                            assembly_code << "    xor rdx, rdx\n";
                            assembly_code << "    div rbx\n";
                            assembly_code << "    add dl, '0'\n";
                            assembly_code << "    mov [r13 + r12], dl\n";
                            assembly_code << "    inc r12\n";
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .digit_loop_" << label_counter << "\n";
                            assembly_code << ".print_loop_" << label_counter << ":\n";
                            assembly_code << "    dec r12\n";
                            assembly_code << "    movzx rcx, byte [r13 + r12]\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    test r12, r12\n";
                            assembly_code << "    jnz .print_loop_" << label_counter << "\n";
                            assembly_code << ".done_print_" << label_counter << ":\n";
                            assembly_code << "    mov rcx, 10\n";
                            assembly_code << "    call putchar\n";
                            label_counter++;
                        }
                        j += 3;
                    }
                }
                
                assembly_code << "    jmp " << loop_start << "\n";
                assembly_code << loop_end << ":\n\n";
            }
        }
        // Handle if statement: if begin condition end front body back
        else if (tokens[i].type == TokenType::_if &&
                 i + 2 < tokens.size() &&
                 tokens[i + 1].type == TokenType::open_paren) {
            
            std::string if_end = generate_label("if_end_");
            
            i += 2; // Skip 'if' and 'begin'
            
            // Collect condition tokens
            std::vector<Token> condition_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::close_paren) {
                condition_tokens.push_back(tokens[i]);
                i++;
            }
            i++; // Skip 'end'
            
            if (i < tokens.size() && tokens[i].type == TokenType::open_brace) {
                i++; // Skip 'front'
                
                // Generate condition check
                std::string jump_instr = generate_condition(condition_tokens, scalars, assembly_code);
                assembly_code << "    " << jump_instr << " " << if_end << "\n\n";
                
                // Process if body - collect body tokens
                std::vector<Token> body_tokens;
                int brace_depth = 1;
                while (i < tokens.size() && brace_depth > 0) {
                    if (tokens[i].type == TokenType::open_brace) brace_depth++;
                    if (tokens[i].type == TokenType::close_brace) {
                        brace_depth--;
                        if (brace_depth == 0) break;
                    }
                    body_tokens.push_back(tokens[i]);
                    i++;
                }
                
                // Process body tokens inline
                for (size_t j = 0; j < body_tokens.size(); j++) {
                    // Handle scalar assignment in if body
                    if (j + 2 < body_tokens.size() &&
                        body_tokens[j].type == TokenType::identifier &&
                        body_tokens[j + 1].type == TokenType::equals &&
                        body_tokens[j + 2].type == TokenType::int_lit) {
                        
                        std::string var_name = body_tokens[j].value.value();
                        int value = base17_to_decimal(body_tokens[j + 2].value.value());
                        
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    mov qword [rbp" << offset << "], " << value << "\n";
                        }
                        j += 2;
                    }
                    // Handle tlc() in if body
                    else if (j + 3 < body_tokens.size() &&
                             body_tokens[j].type == TokenType::_tlc &&
                             body_tokens[j + 1].type == TokenType::open_paren &&
                             body_tokens[j + 2].type == TokenType::identifier &&
                             body_tokens[j + 3].type == TokenType::close_paren) {
                        
                        std::string var_name = body_tokens[j + 2].value.value();
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    ; Print variable " << var_name << "\n";
                            assembly_code << "    mov rax, [rbp" << offset << "]\n";
                            
                            // Special case for zero
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .not_zero_" << label_counter << "\n";
                            assembly_code << "    mov rcx, '0'\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    jmp .done_print_" << label_counter << "\n";
                            assembly_code << ".not_zero_" << label_counter << ":\n";
                            
                            // Convert to string and print (store in buffer)
                            assembly_code << "    lea r13, [rel temp_buffer]\n";
                            assembly_code << "    mov rbx, 10\n";
                            assembly_code << "    xor r12, r12\n";
                            assembly_code << ".digit_loop_" << label_counter << ":\n";
                            assembly_code << "    xor rdx, rdx\n";
                            assembly_code << "    div rbx\n";
                            assembly_code << "    add dl, '0'\n";
                            assembly_code << "    mov [r13 + r12], dl\n";
                            assembly_code << "    inc r12\n";
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .digit_loop_" << label_counter << "\n";
                            assembly_code << ".print_loop_" << label_counter << ":\n";
                            assembly_code << "    dec r12\n";
                            assembly_code << "    movzx rcx, byte [r13 + r12]\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    test r12, r12\n";
                            assembly_code << "    jnz .print_loop_" << label_counter << "\n";
                            assembly_code << ".done_print_" << label_counter << ":\n";
                            assembly_code << "    mov rcx, 10\n";
                            assembly_code << "    call putchar\n";
                            label_counter++;
                        }
                        j += 3;
                    }
                }
                
                assembly_code << if_end << ":\n\n";
            }
        }
        // Handle for loop: for begin init. condition. increment end front body back
        else if (tokens[i].type == TokenType::_for &&
                 i + 2 < tokens.size() &&
                 tokens[i + 1].type == TokenType::open_paren) {
            
            std::string loop_start = generate_label("for_start_");
            std::string loop_end = generate_label("for_end_");
            
            i += 2; // Skip 'for' and 'begin'
            
            // Parse initialization (until first semicolon)
            std::vector<Token> init_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::semi) {
                init_tokens.push_back(tokens[i]);
                i++;
            }
            i++; // Skip semicolon
            
            // Parse condition (until second semicolon)
            std::vector<Token> condition_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::semi) {
                condition_tokens.push_back(tokens[i]);
                i++;
            }
            i++; // Skip semicolon
            
            // Parse increment (until 'end')
            std::vector<Token> increment_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::close_paren) {
                increment_tokens.push_back(tokens[i]);
                i++;
            }
            i++; // Skip 'end'
            
            // Process initialization
            if (init_tokens.size() >= 3 &&
                init_tokens[0].type == TokenType::identifier &&
                init_tokens[1].type == TokenType::equals &&
                init_tokens[2].type == TokenType::int_lit) {
                
                std::string var_name = init_tokens[0].value.value();
                int value = base17_to_decimal(init_tokens[2].value.value());
                
                if (scalars.find(var_name) == scalars.end()) {
                    stack_offset -= 8;
                    scalars[var_name] = stack_offset;
                }
                
                int offset = scalars[var_name];
                assembly_code << "    ; Initialize " << var_name << " = " << value << "\n";
                assembly_code << "    mov qword [rbp" << offset << "], " << value << "\n\n";
            }
            
            if (i < tokens.size() && tokens[i].type == TokenType::open_brace) {
                i++; // Skip 'front'
                
                assembly_code << loop_start << ":\n";
                
                // Generate condition check
                std::string jump_instr = generate_condition(condition_tokens, scalars, assembly_code);
                assembly_code << "    " << jump_instr << " " << loop_end << "\n\n";
                
                // Process loop body - collect body tokens
                std::vector<Token> body_tokens;
                int brace_depth = 1;
                while (i < tokens.size() && brace_depth > 0) {
                    if (tokens[i].type == TokenType::open_brace) brace_depth++;
                    if (tokens[i].type == TokenType::close_brace) {
                        brace_depth--;
                        if (brace_depth == 0) break;
                    }
                    body_tokens.push_back(tokens[i]);
                    i++;
                }
                
                // Process body tokens (similar to while loop)
                for (size_t j = 0; j < body_tokens.size(); j++) {
                    // Handle if statement in for loop body
                    if (body_tokens[j].type == TokenType::_if &&
                        j + 2 < body_tokens.size() &&
                        body_tokens[j + 1].type == TokenType::open_paren) {
                        
                        std::string if_end = generate_label("if_end_");
                        
                        j += 2; // Skip 'if' and 'begin'
                        
                        // Collect condition tokens
                        std::vector<Token> if_condition_tokens;
                        while (j < body_tokens.size() && body_tokens[j].type != TokenType::close_paren) {
                            if_condition_tokens.push_back(body_tokens[j]);
                            j++;
                        }
                        j++; // Skip 'end'
                        
                        if (j < body_tokens.size() && body_tokens[j].type == TokenType::open_brace) {
                            j++; // Skip 'front'
                            
                            // Generate condition check
                            std::string jump_instr = generate_condition(if_condition_tokens, scalars, assembly_code);
                            assembly_code << "    " << jump_instr << " " << if_end << "\n\n";
                            
                            // Collect if body tokens
                            std::vector<Token> if_body_tokens;
                            int if_brace_depth = 1;
                            while (j < body_tokens.size() && if_brace_depth > 0) {
                                if (body_tokens[j].type == TokenType::open_brace) if_brace_depth++;
                                if (body_tokens[j].type == TokenType::close_brace) {
                                    if_brace_depth--;
                                    if (if_brace_depth == 0) break;
                                }
                                if_body_tokens.push_back(body_tokens[j]);
                                j++;
                            }
                            
                            // Process if body (handle tlc statements)
                            for (size_t k = 0; k < if_body_tokens.size(); k++) {
                                if (k + 3 < if_body_tokens.size() &&
                                    if_body_tokens[k].type == TokenType::_tlc &&
                                    if_body_tokens[k + 1].type == TokenType::open_paren &&
                                    if_body_tokens[k + 2].type == TokenType::identifier &&
                                    if_body_tokens[k + 3].type == TokenType::close_paren) {
                                    
                                    std::string var_name = if_body_tokens[k + 2].value.value();
                                    if (scalars.find(var_name) != scalars.end()) {
                                        int offset = scalars[var_name];
                                        assembly_code << "    ; Print variable " << var_name << "\n";
                                        assembly_code << "    mov rax, [rbp" << offset << "]\n";
                                        
                                        // Special case for zero
                                        assembly_code << "    test rax, rax\n";
                                        assembly_code << "    jnz .not_zero_" << label_counter << "\n";
                                        assembly_code << "    mov rcx, '0'\n";
                                        assembly_code << "    call putchar\n";
                                        assembly_code << "    jmp .done_print_" << label_counter << "\n";
                                        assembly_code << ".not_zero_" << label_counter << ":\n";
                                        
                                        // Convert to string and print (store in buffer)
                                        assembly_code << "    lea r13, [rel temp_buffer]\n";
                                        assembly_code << "    mov rbx, 10\n";
                                        assembly_code << "    xor r12, r12\n";
                                        assembly_code << ".digit_loop_" << label_counter << ":\n";
                                        assembly_code << "    xor rdx, rdx\n";
                                        assembly_code << "    div rbx\n";
                                        assembly_code << "    add dl, '0'\n";
                                        assembly_code << "    mov [r13 + r12], dl\n";
                                        assembly_code << "    inc r12\n";
                                        assembly_code << "    test rax, rax\n";
                                        assembly_code << "    jnz .digit_loop_" << label_counter << "\n";
                                        assembly_code << ".print_loop_" << label_counter << ":\n";
                                        assembly_code << "    dec r12\n";
                                        assembly_code << "    movzx rcx, byte [r13 + r12]\n";
                                        assembly_code << "    call putchar\n";
                                        assembly_code << "    test r12, r12\n";
                                        assembly_code << "    jnz .print_loop_" << label_counter << "\n";
                                        assembly_code << ".done_print_" << label_counter << ":\n";
                                        assembly_code << "    mov rcx, 10\n";
                                        assembly_code << "    call putchar\n";
                                        label_counter++;
                                    }
                                    k += 3;
                                }
                            }
                            
                            assembly_code << if_end << ":\n";
                        }
                    }
                    // Handle tlc() in loop
                    else if (j + 3 < body_tokens.size() &&
                        body_tokens[j].type == TokenType::_tlc &&
                        body_tokens[j + 1].type == TokenType::open_paren &&
                        body_tokens[j + 2].type == TokenType::identifier &&
                        body_tokens[j + 3].type == TokenType::close_paren) {
                        
                        std::string var_name = body_tokens[j + 2].value.value();
                        if (scalars.find(var_name) != scalars.end()) {
                            int offset = scalars[var_name];
                            assembly_code << "    ; Print variable " << var_name << "\n";
                            assembly_code << "    mov rax, [rbp" << offset << "]\n";
                            
                            // Special case for zero
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .not_zero_" << label_counter << "\n";
                            assembly_code << "    mov rcx, '0'\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    jmp .done_print_" << label_counter << "\n";
                            assembly_code << ".not_zero_" << label_counter << ":\n";
                            
                            // Convert to string and print (store in buffer to avoid stack issues)
                            assembly_code << "    lea r13, [rel temp_buffer]\n";
                            assembly_code << "    mov rbx, 10\n";
                            assembly_code << "    xor r12, r12\n";
                            assembly_code << ".digit_loop_" << label_counter << ":\n";
                            assembly_code << "    xor rdx, rdx\n";
                            assembly_code << "    div rbx\n";
                            assembly_code << "    add dl, '0'\n";
                            assembly_code << "    mov [r13 + r12], dl\n";
                            assembly_code << "    inc r12\n";
                            assembly_code << "    test rax, rax\n";
                            assembly_code << "    jnz .digit_loop_" << label_counter << "\n";
                            assembly_code << ".print_loop_" << label_counter << ":\n";
                            assembly_code << "    dec r12\n";
                            assembly_code << "    movzx rcx, byte [r13 + r12]\n";
                            assembly_code << "    call putchar\n";
                            assembly_code << "    test r12, r12\n";
                            assembly_code << "    jnz .print_loop_" << label_counter << "\n";
                            assembly_code << ".done_print_" << label_counter << ":\n";
                            assembly_code << "    mov rcx, 10\n";
                            assembly_code << "    call putchar\n";
                            label_counter++;
                        }
                        j += 3;
                    }
                }
                
                // Process increment
                if (increment_tokens.size() >= 5 &&
                    increment_tokens[0].type == TokenType::identifier &&
                    increment_tokens[1].type == TokenType::equals &&
                    (increment_tokens[3].type == TokenType::_durham ||
                     increment_tokens[3].type == TokenType::_newcastle)) {
                    
                    std::string var_name = increment_tokens[0].value.value();
                    
                    // Load left operand
                    if (increment_tokens[2].type == TokenType::identifier) {
                        std::string left_var = increment_tokens[2].value.value();
                        if (scalars.find(left_var) != scalars.end()) {
                            assembly_code << "    mov rax, [rbp" << scalars[left_var] << "]\n";
                        }
                    }
                    
                    // Load right operand
                    if (increment_tokens[4].type == TokenType::int_lit) {
                        int val = base17_to_decimal(increment_tokens[4].value.value());
                        assembly_code << "    mov rbx, " << val << "\n";
                    }
                    
                    // Perform operation
                    if (increment_tokens[3].type == TokenType::_durham) {
                        assembly_code << "    add rax, rbx\n";
                    } else if (increment_tokens[3].type == TokenType::_newcastle) {
                        assembly_code << "    sub rax, rbx\n";
                    }
                    
                    if (scalars.find(var_name) != scalars.end()) {
                        int offset = scalars[var_name];
                        assembly_code << "    mov [rbp" << offset << "], rax\n";
                    }
                }
                
                assembly_code << "    jmp " << loop_start << "\n";
                assembly_code << loop_end << ":\n\n";
            }
        }
        // Handle tlc(identifier) for vectors
        else if (i + 3 < tokens.size() &&
                 tokens[i].type == TokenType::_tlc &&
                 tokens[i + 1].type == TokenType::open_paren &&
                 tokens[i + 2].type == TokenType::identifier &&
                 tokens[i + 3].type == TokenType::close_paren) {
            
            std::string var_name = tokens[i + 2].value.value();
            
            // Check if it's a vector
            if (variables.find(var_name) != variables.end()) {
                int var_offset = variables[var_name];
                
                assembly_code << "    ; Print vector '" << var_name << "'\n";
                assembly_code << "    mov r10, [rbp" << var_offset << "]  ; load size\n";
                assembly_code << "    lea r11, [rbp" << (var_offset - 8) << "]  ; array start\n";
                
                // Print '('
                assembly_code << "    mov rcx, 40\n";
                assembly_code << "    call putchar\n";
                
                // Print elements
                assembly_code << "    xor r12, r12\n";
                assembly_code << ".print_loop_" << var_name << ":\n";
                assembly_code << "    cmp r12, r10\n";
                assembly_code << "    jge .print_done_" << var_name << "\n";
                
                assembly_code << "    mov rax, [r11 + r12*8]\n";
                assembly_code << "    add rax, 48\n";
                assembly_code << "    mov rcx, rax\n";
                assembly_code << "    call putchar\n";
                
                assembly_code << "    inc r12\n";
                assembly_code << "    cmp r12, r10\n";
                assembly_code << "    jge .print_done_" << var_name << "\n";
                assembly_code << "    mov rcx, 44\n";
                assembly_code << "    call putchar\n";
                assembly_code << "    jmp .print_loop_" << var_name << "\n";
                
                assembly_code << ".print_done_" << var_name << ":\n";
                assembly_code << "    mov rcx, 41\n";
                assembly_code << "    call putchar\n";
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
            }
            // Check if it's a scalar
            else if (scalars.find(var_name) != scalars.end()) {
                int offset = scalars[var_name];
                
                assembly_code << "    ; Print scalar '" << var_name << "'\n";
                assembly_code << "    mov rax, [rbp" << offset << "]\n";
                
                // Special case for zero
                assembly_code << "    test rax, rax\n";
                assembly_code << "    jnz .not_zero_" << label_counter << "\n";
                assembly_code << "    mov rcx, '0'\n";
                assembly_code << "    call putchar\n";
                assembly_code << "    jmp .done_print_" << label_counter << "\n";
                assembly_code << ".not_zero_" << label_counter << ":\n";
                
                // Convert number to string and print (store in buffer)
                assembly_code << "    lea r13, [rel temp_buffer]\n";
                assembly_code << "    mov rbx, 10\n";
                assembly_code << "    xor r12, r12\n";
                assembly_code << ".digit_loop_" << label_counter << ":\n";
                assembly_code << "    xor rdx, rdx\n";
                assembly_code << "    div rbx\n";
                assembly_code << "    add dl, '0'\n";
                assembly_code << "    mov [r13 + r12], dl\n";
                assembly_code << "    inc r12\n";
                assembly_code << "    test rax, rax\n";
                assembly_code << "    jnz .digit_loop_" << label_counter << "\n";
                assembly_code << ".print_loop_" << label_counter << ":\n";
                assembly_code << "    dec r12\n";
                assembly_code << "    movzx rcx, byte [r13 + r12]\n";
                assembly_code << "    call putchar\n";
                assembly_code << "    test r12, r12\n";
                assembly_code << "    jnz .print_loop_" << label_counter << "\n";
                assembly_code << ".done_print_" << label_counter << ":\n";
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
                label_counter++;
            }
            else {
                std::cerr << "Error: Variable '" << var_name << "' not defined" << std::endl;
            }
            
            i += 3; // Skip tlc, (, identifier, )
        }
        // Handle tlc(expression) for direct calculations
        else if (tokens[i].type == TokenType::_tlc && 
                 i + 1 < tokens.size() &&
                 tokens[i + 1].type == TokenType::open_paren) {
            
            i += 2; // Skip tlc and (
            
            std::vector<Token> expr_tokens;
            while (i < tokens.size() && tokens[i].type != TokenType::close_paren) {
                expr_tokens.push_back(tokens[i]);
                i++;
            }
            
            // Evaluate expression
            if (expr_tokens.size() == 3 &&
                expr_tokens[0].type == TokenType::int_lit &&
                expr_tokens[2].type == TokenType::int_lit) {
                
                int left = base17_to_decimal(expr_tokens[0].value.value());
                int right = base17_to_decimal(expr_tokens[2].value.value());
                int result = 0;
                
                if (expr_tokens[1].type == TokenType::_durham) {
                    result = left + right;  // +
                } else if (expr_tokens[1].type == TokenType::_newcastle) {
                    result = left - right;  // -
                } else if (expr_tokens[1].type == TokenType::_york) {
                    result = left * right;  // *
                } else if (expr_tokens[1].type == TokenType::_edinburgh) {
                    result = left / right;  // /
                }
                
                std::string result_str = std::to_string(result);
                for (char digit_char : result_str) {
                    assembly_code << "    mov rcx, " << static_cast<int>(digit_char) << "\n";
                    assembly_code << "    call putchar\n";
                }
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
                
            } else if (expr_tokens.size() == 1 && expr_tokens[0].type == TokenType::int_lit) {
                // Convert base-17 string to decimal
                int decimal_value = base17_to_decimal(expr_tokens[0].value.value());
                
                // Print decimal value
                std::string result_str = std::to_string(decimal_value);
                for (char digit_char : result_str) {
                    assembly_code << "    mov rcx, " << static_cast<int>(digit_char) << "\n";
                    assembly_code << "    call putchar\n";
                }
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
            }
        }
    }

    assembly_code << "    xor rax, rax\n";
    assembly_code << "    add rsp, 1024\n";
    assembly_code << "    pop rbp\n";
    assembly_code << "    ret\n";

    return assembly_code.str(); 
}

