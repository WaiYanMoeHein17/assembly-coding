#include "main.h"
#include "gen_asm.h"

// Forward declarations for AST-based code generation
void generate_node(std::shared_ptr<ASTNode> node, 
                   std::stringstream& asm_code,
                   std::map<std::string, int>& var_offsets,
                   int& stack_offset,
                   int& label_counter);

void generate_expression(std::shared_ptr<ASTNode> node,
                        std::stringstream& asm_code,
                        std::map<std::string, int>& var_offsets);

void generate_condition(std::shared_ptr<ASTNode> node,
                       std::stringstream& asm_code,
                       std::map<std::string, int>& var_offsets,
                       int label,
                       const std::string& label_prefix = "while");

// Helper function to check if an expression is a string type
bool is_string_expression(std::shared_ptr<ASTNode> node, const std::map<std::string, std::string>& string_vars);

// Helper function to generate string concatenation
void generate_string_concat(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right,
                           std::stringstream& asm_code,
                           std::map<std::string, int>& var_offsets,
                           const std::map<std::string, std::string>& string_vars,
                           const std::map<std::string, int>& string_lits);

// Helper function to convert decimal string to integer
// Now that we use decimal (not base-17), this is just a wrapper around stoi
int base17_to_decimal(const std::string& decimal_str) {
    return std::stoi(decimal_str);
}

// Helper function to generate unique labels
static int label_counter = 0;
std::string generate_label(const std::string& prefix) {
    return prefix + std::to_string(label_counter++);
}

// Helper function to evaluate a condition and generate comparison code at runtime (token-based)
std::string generate_condition_from_tokens(const std::vector<Token>& condition_tokens, 
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
                std::string jump_instr = generate_condition_from_tokens(condition_tokens, scalars, assembly_code);
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
                std::string jump_instr = generate_condition_from_tokens(condition_tokens, scalars, assembly_code);
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
                std::string jump_instr = generate_condition_from_tokens(condition_tokens, scalars, assembly_code);
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
                            std::string jump_instr = generate_condition_from_tokens(if_condition_tokens, scalars, assembly_code);
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

// Add this new function at the top after the includes
// Helper to collect string literals from AST
static std::map<std::string, int> string_literals;
static int string_counter = 0;
static std::map<std::string, std::string> string_variables; // varname -> string label

// Helper function to check if an expression is a string type
bool is_string_expression(std::shared_ptr<ASTNode> node, const std::map<std::string, std::string>& string_vars) {
    if (!node) return false;
    
    if (node->type == NodeType::StringLiteral) return true;
    
    if (node->type == NodeType::Identifier) {
        std::string var_name = node->value.value();
        return string_vars.find(var_name) != string_vars.end();
    }
    
    if (node->type == NodeType::BinaryOp) {
        auto binOp = std::static_pointer_cast<BinaryOpNode>(node);
        if (binOp->op == TokenType::_durham) {
            return is_string_expression(binOp->left, string_vars) || 
                   is_string_expression(binOp->right, string_vars);
        }
    }
    
    return false;
}

// Helper function to generate string concatenation
// This allocates heap memory for the concatenated result
void generate_string_concat(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right,
                           std::stringstream& asm_code,
                           std::map<std::string, int>& var_offsets,
                           const std::map<std::string, std::string>& string_vars,
                           const std::map<std::string, int>& string_lits) {
    static int concat_counter = 0;
    int current_concat = concat_counter++;
    
    asm_code << "    ; String concatenation\n";
    
    // Get left string pointer into r12
    if (left->type == NodeType::StringLiteral) {
        std::string str = left->value.value();
        int str_id = string_lits.at(str);
        asm_code << "    lea r12, [rel str_" << str_id << "]\n";
    } else if (left->type == NodeType::Identifier) {
        std::string var_name = left->value.value();
        asm_code << "    mov r12, [rbp-" << var_offsets.at(var_name) << "]\n";
    } else if (left->type == NodeType::BinaryOp) {
        // Recursively handle nested concatenation
        auto leftBinOp = std::static_pointer_cast<BinaryOpNode>(left);
        if (leftBinOp->op == TokenType::_durham) {
            generate_string_concat(leftBinOp->left, leftBinOp->right, asm_code, 
                                 var_offsets, string_vars, string_lits);
            asm_code << "    mov r12, rax\n";
        }
    }
    
    // Get right string pointer into r13
    if (right->type == NodeType::StringLiteral) {
        std::string str = right->value.value();
        int str_id = string_lits.at(str);
        asm_code << "    lea r13, [rel str_" << str_id << "]\n";
    } else if (right->type == NodeType::Identifier) {
        std::string var_name = right->value.value();
        asm_code << "    mov r13, [rbp-" << var_offsets.at(var_name) << "]\n";
    } else if (right->type == NodeType::BinaryOp) {
        // Recursively handle nested concatenation
        auto rightBinOp = std::static_pointer_cast<BinaryOpNode>(right);
        if (rightBinOp->op == TokenType::_durham) {
            generate_string_concat(rightBinOp->left, rightBinOp->right, asm_code, 
                                 var_offsets, string_vars, string_lits);
            asm_code << "    mov r13, rax\n";
        }
    }
    
    // Calculate left string length
    asm_code << "    mov r14, r12\n";
    asm_code << "    xor r15, r15\n";
    asm_code << ".strlen_left_" << current_concat << ":\n";
    asm_code << "    movzx rax, byte [r14]\n";
    asm_code << "    test rax, rax\n";
    asm_code << "    jz .done_strlen_left_" << current_concat << "\n";
    asm_code << "    inc r15\n";
    asm_code << "    inc r14\n";
    asm_code << "    jmp .strlen_left_" << current_concat << "\n";
    asm_code << ".done_strlen_left_" << current_concat << ":\n";
    
    // Calculate right string length
    asm_code << "    mov r14, r13\n";
    asm_code << "    xor rbx, rbx\n";
    asm_code << ".strlen_right_" << current_concat << ":\n";
    asm_code << "    movzx rax, byte [r14]\n";
    asm_code << "    test rax, rax\n";
    asm_code << "    jz .done_strlen_right_" << current_concat << "\n";
    asm_code << "    inc rbx\n";
    asm_code << "    inc r14\n";
    asm_code << "    jmp .strlen_right_" << current_concat << "\n";
    asm_code << ".done_strlen_right_" << current_concat << ":\n";
    
    // Allocate heap memory: left_len + right_len + 1 (for null terminator)
    asm_code << "    mov rax, r15\n";
    asm_code << "    add rax, rbx\n";
    asm_code << "    inc rax\n";  // +1 for null terminator
    asm_code << "    mov rcx, [rel heap_ptr]\n";
    asm_code << "    mov r14, rcx\n";  // Save start pointer
    asm_code << "    add rcx, rax\n";
    asm_code << "    mov [rel heap_ptr], rcx\n";
    
    // Copy left string
    asm_code << "    mov rsi, r12\n";
    asm_code << "    mov rdi, r14\n";
    asm_code << ".copy_left_" << current_concat << ":\n";
    asm_code << "    movzx rax, byte [rsi]\n";
    asm_code << "    test rax, rax\n";
    asm_code << "    jz .done_copy_left_" << current_concat << "\n";
    asm_code << "    mov [rdi], al\n";
    asm_code << "    inc rsi\n";
    asm_code << "    inc rdi\n";
    asm_code << "    jmp .copy_left_" << current_concat << "\n";
    asm_code << ".done_copy_left_" << current_concat << ":\n";
    
    // Copy right string
    asm_code << "    mov rsi, r13\n";
    asm_code << ".copy_right_" << current_concat << ":\n";
    asm_code << "    movzx rax, byte [rsi]\n";
    asm_code << "    mov [rdi], al\n";
    asm_code << "    test rax, rax\n";
    asm_code << "    jz .done_copy_right_" << current_concat << "\n";
    asm_code << "    inc rsi\n";
    asm_code << "    inc rdi\n";
    asm_code << "    jmp .copy_right_" << current_concat << "\n";
    asm_code << ".done_copy_right_" << current_concat << ":\n";
    
    // Return pointer to concatenated string in rax
    asm_code << "    mov rax, r14\n";
}

void collect_strings(std::shared_ptr<ASTNode> node) {
    if (!node) return;
    
    if (node->type == NodeType::Print && node->value.has_value()) {
        // This is a string print
        std::string str = node->value.value();
        if (string_literals.find(str) == string_literals.end()) {
            string_literals[str] = string_counter++;
        }
    }
    
    // Collect string literals from StringLiteral nodes
    if (node->type == NodeType::StringLiteral && node->value.has_value()) {
        std::string str = node->value.value();
        if (string_literals.find(str) == string_literals.end()) {
            string_literals[str] = string_counter++;
        }
    }
    
    // Handle ForLoop nodes specially (they have init, condition, increment, body)
    if (node->type == NodeType::ForLoop) {
        auto forNode = std::static_pointer_cast<ForNode>(node);
        collect_strings(forNode->init);
        collect_strings(forNode->condition);
        collect_strings(forNode->increment);
        collect_strings(forNode->body);
    }
    
    // Handle IfStatement nodes (they have condition, thenBranch, elseBranch)
    if (node->type == NodeType::IfStatement) {
        auto ifNode = std::static_pointer_cast<IfNode>(node);
        collect_strings(ifNode->condition);
        collect_strings(ifNode->thenBranch);
        collect_strings(ifNode->elseBranch);
    }
    
    // Handle WhileLoop nodes (they have condition and body)
    if (node->type == NodeType::WhileLoop) {
        auto whileNode = std::static_pointer_cast<WhileNode>(node);
        collect_strings(whileNode->condition);
        collect_strings(whileNode->body);
    }
    
    // Handle FunctionDecl nodes (they have parameters and body)
    if (node->type == NodeType::FunctionDecl) {
        auto funcNode = std::static_pointer_cast<FunctionDeclNode>(node);
        collect_strings(funcNode->body);
    }
    
    // Recursively check standard children
    if (node->left) collect_strings(node->left);
    if (node->right) collect_strings(node->right);
    for (auto& child : node->children) {
        collect_strings(child);
    }
}

std::string generate_assembly_from_ast(std::shared_ptr<ASTNode> ast) {
    std::stringstream asm_code;
    
    // Reset and collect string literals
    string_literals.clear();
    string_counter = 0;
    collect_strings(ast);
    
    // Header
    asm_code << "section .data\n";
    asm_code << "    digit db '0', 10\n";
    asm_code << "    array times 1000 dq 0\n";
    
    // Add string literals
    for (const auto& [str, id] : string_literals) {
        asm_code << "    str_" << id << " db \"" << str << "\", 0\n";
    }
    asm_code << "\n";
    
    asm_code << "section .bss\n";
    asm_code << "    temp_buffer resb 32\n";
    asm_code << "    heap_space resb 8192\n";  // 8KB heap for vectors
    asm_code << "    heap_ptr resq 1\n\n";     // Pointer to next free space
    
    asm_code << "section .text\n";
    asm_code << "    global main\n";
    asm_code << "    extern putchar\n\n";
    
    // First pass: Generate function declarations
    if (ast->type == NodeType::Program) {
        for (auto& child : ast->children) {
            if (child->type == NodeType::FunctionDecl) {
                std::map<std::string, int> dummy_vars;
                int dummy_stack = 0;
                int dummy_label = 0;
                generate_node(child, asm_code, dummy_vars, dummy_stack, dummy_label);
            }
        }
    }
    
    asm_code << "main:\n";
    asm_code << "    push rbp\n";
    asm_code << "    mov rbp, rsp\n";
    asm_code << "    sub rsp, 1024\n\n";
    asm_code << "    ; Initialize heap pointer\n";
    asm_code << "    lea rax, [rel heap_space]\n";
    asm_code << "    mov [rel heap_ptr], rax\n\n";
    
    // State for code generation
    std::map<std::string, int> var_offsets;
    int stack_offset = 0;
    int label_counter = 0;
    
    // Second pass: Generate non-function statements for main
    if (ast->type == NodeType::Program) {
        for (auto& child : ast->children) {
            if (child->type != NodeType::FunctionDecl) {
                generate_node(child, asm_code, var_offsets, stack_offset, label_counter);
            }
        }
    } else {
        // Not a program node, generate directly
        generate_node(ast, asm_code, var_offsets, stack_offset, label_counter);
    }
    
    // Footer
    asm_code << "\n    xor eax, eax\n";
    asm_code << "    add rsp, 1024\n";
    asm_code << "    pop rbp\n";
    asm_code << "    ret\n";
    
    return asm_code.str();
}

// Helper function to generate code for a single node
void generate_node(std::shared_ptr<ASTNode> node, 
                   std::stringstream& asm_code,
                   std::map<std::string, int>& var_offsets,
                   int& stack_offset,
                   int& label_counter) {
    if (!node) return;
    
    switch (node->type) {
        case NodeType::Program: {
            // Process all statements in the program
            for (auto& child : node->children) {
                generate_node(child, asm_code, var_offsets, stack_offset, label_counter);
            }
            break;
        }
        
        case NodeType::Block: {
            // Process all statements in the block
            for (auto& child : node->children) {
                generate_node(child, asm_code, var_offsets, stack_offset, label_counter);
            }
            break;
        }
        
        case NodeType::Assignment: {
            auto assignNode = std::static_pointer_cast<AssignmentNode>(node);
            std::string var_name = assignNode->varName;
            std::string var_type = assignNode->varType;
            
            // Check if this is array element assignment (left side is ArrayAccess)
            if (assignNode->left && assignNode->left->type == NodeType::ArrayAccess) {
                auto accessNode = std::static_pointer_cast<ArrayAccessNode>(assignNode->left);
                
                // Get the array pointer
                asm_code << "    mov rbx, [rbp-" << var_offsets[accessNode->arrayName] << "]\n";
                
                // Evaluate index and save it
                generate_expression(accessNode->index, asm_code, var_offsets);
                asm_code << "    push rax\n";
                
                // Evaluate the value to assign
                generate_expression(assignNode->right, asm_code, var_offsets);
                asm_code << "    mov rcx, rax\n";  // Save value in rcx
                
                // Calculate offset
                asm_code << "    pop rax\n";  // Get index back
                asm_code << "    imul rax, 8\n";  // index * 8
                asm_code << "    add rbx, rax\n";  // array + offset
                asm_code << "    mov [rbx], rcx\n";  // Store value
            } else if (var_type == "text") {
                // String variable assignment
                
                // Allocate space on stack for the pointer
                if (var_offsets.find(var_name) == var_offsets.end()) {
                    stack_offset += 8;
                    var_offsets[var_name] = stack_offset;
                }
                
                // Track this as a string variable (mark it before generating expression)
                string_variables[var_name] = var_name;  // Mark as string variable
                
                // Generate the expression (could be literal, variable, or concatenation)
                // Result will be a pointer in rax
                if (assignNode->right->type == NodeType::StringLiteral) {
                    std::string str_value = assignNode->right->value.value();
                    int str_id = string_literals[str_value];
                    asm_code << "    lea rax, [rel str_" << str_id << "]\n";
                } else {
                    // Could be another string variable or concatenation
                    generate_expression(assignNode->right, asm_code, var_offsets);
                }
                
                // Store pointer to string
                asm_code << "    mov [rbp-" << var_offsets[var_name] << "], rax\n";
            } else {
                // Regular numeric variable assignment
                
                // Allocate space on stack if new variable
                if (var_offsets.find(var_name) == var_offsets.end()) {
                    stack_offset += 8;
                    var_offsets[var_name] = stack_offset;
                }
                
                // Generate code for the expression
                generate_expression(assignNode->right, asm_code, var_offsets);
                
                // Store result in variable
                asm_code << "    mov [rbp-" << var_offsets[var_name] << "], rax\n";
            }
            break;
        }
        
        case NodeType::Print: {
            // Check if it's a string literal print
            if (node->value.has_value()) {
                std::string str = node->value.value();
                int str_id = string_literals[str];
                
                asm_code << "    ; Print string\n";
                asm_code << "    lea rbx, [rel str_" << str_id << "]\n";
                asm_code << ".print_str_" << label_counter << ":\n";
                asm_code << "    movzx rcx, byte [rbx]\n";
                asm_code << "    test rcx, rcx\n";
                asm_code << "    jz .done_str_" << label_counter << "\n";
                asm_code << "    call putchar\n";
                asm_code << "    inc rbx\n";
                asm_code << "    jmp .print_str_" << label_counter << "\n";
                asm_code << ".done_str_" << label_counter << ":\n";
                asm_code << "    mov rcx, 10\n";  // Print newline
                asm_code << "    call putchar\n";
                
                label_counter++;
            } else if (node->left && node->left->type == NodeType::Identifier) {
                // Check if this is a string variable
                std::string var_name = node->left->value.value();
                if (string_variables.find(var_name) != string_variables.end()) {
                    // Print string variable
                    asm_code << "    ; Print string variable\n";
                    asm_code << "    mov rbx, [rbp-" << var_offsets[var_name] << "]\n";
                    asm_code << ".print_str_" << label_counter << ":\n";
                    asm_code << "    movzx rcx, byte [rbx]\n";
                    asm_code << "    test rcx, rcx\n";
                    asm_code << "    jz .done_str_" << label_counter << "\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    inc rbx\n";
                    asm_code << "    jmp .print_str_" << label_counter << "\n";
                    asm_code << ".done_str_" << label_counter << ":\n";
                    asm_code << "    mov rcx, 10\n";  // Print newline
                    asm_code << "    call putchar\n";
                    
                    label_counter++;
                } else {
                    // Print numeric variable
                    generate_expression(node->left, asm_code, var_offsets);
                    
                    // Print the value in rax
                    asm_code << "    ; Print value in rax\n";
                    asm_code << "    test rax, rax\n";
                    asm_code << "    jnz .not_zero_" << label_counter << "\n";
                    asm_code << "    mov rcx, '0'\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    jmp .done_print_" << label_counter << "\n";
                    asm_code << ".not_zero_" << label_counter << ":\n";
                    asm_code << "    mov rbx, 10\n";
                    asm_code << "    xor r12, r12\n";
                    asm_code << "    lea r13, [rel temp_buffer]\n";
                    asm_code << ".digit_loop_" << label_counter << ":\n";
                    asm_code << "    xor rdx, rdx\n";
                    asm_code << "    div rbx\n";
                    asm_code << "    add dl, '0'\n";
                    asm_code << "    mov [r13 + r12], dl\n";
                    asm_code << "    inc r12\n";
                    asm_code << "    test rax, rax\n";
                    asm_code << "    jnz .digit_loop_" << label_counter << "\n";
                    asm_code << ".print_loop_" << label_counter << ":\n";
                    asm_code << "    dec r12\n";
                    asm_code << "    movzx rcx, byte [r13 + r12]\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    test r12, r12\n";
                    asm_code << "    jnz .print_loop_" << label_counter << "\n";
                    asm_code << ".done_print_" << label_counter << ":\n";
                    asm_code << "    mov rcx, 10\n";
                    asm_code << "    call putchar\n";
                    
                    label_counter++;
                }
            } else {
                // Check if this is a string expression (concatenation or string literal)
                if (is_string_expression(node->left, string_variables)) {
                    // Generate code for the string expression
                    generate_expression(node->left, asm_code, var_offsets);
                    
                    // rax now contains pointer to string
                    asm_code << "    ; Print string from expression\n";
                    asm_code << "    mov rbx, rax\n";
                    asm_code << ".print_str_" << label_counter << ":\n";
                    asm_code << "    movzx rcx, byte [rbx]\n";
                    asm_code << "    test rcx, rcx\n";
                    asm_code << "    jz .done_str_" << label_counter << "\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    inc rbx\n";
                    asm_code << "    jmp .print_str_" << label_counter << "\n";
                    asm_code << ".done_str_" << label_counter << ":\n";
                    asm_code << "    mov rcx, 10\n";  // Print newline
                    asm_code << "    call putchar\n";
                    
                    label_counter++;
                } else {
                    // Generate code for the numeric expression to print
                    generate_expression(node->left, asm_code, var_offsets);
                    
                    // Print the value in rax
                    asm_code << "    ; Print value in rax\n";
                    asm_code << "    test rax, rax\n";
                    asm_code << "    jnz .not_zero_" << label_counter << "\n";
                    asm_code << "    mov rcx, '0'\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    jmp .done_print_" << label_counter << "\n";
                    asm_code << ".not_zero_" << label_counter << ":\n";
                    asm_code << "    mov rbx, 10\n";
                    asm_code << "    xor r12, r12\n";
                    asm_code << "    lea r13, [rel temp_buffer]\n";
                    asm_code << ".digit_loop_" << label_counter << ":\n";
                    asm_code << "    xor rdx, rdx\n";
                    asm_code << "    div rbx\n";
                    asm_code << "    add dl, '0'\n";
                    asm_code << "    mov [r13 + r12], dl\n";
                    asm_code << "    inc r12\n";
                    asm_code << "    test rax, rax\n";
                    asm_code << "    jnz .digit_loop_" << label_counter << "\n";
                    asm_code << ".print_loop_" << label_counter << ":\n";
                    asm_code << "    dec r12\n";
                    asm_code << "    movzx rcx, byte [r13 + r12]\n";
                    asm_code << "    call putchar\n";
                    asm_code << "    test r12, r12\n";
                    asm_code << "    jnz .print_loop_" << label_counter << "\n";
                    asm_code << ".done_print_" << label_counter << ":\n";
                    asm_code << "    mov rcx, 10\n";
                    asm_code << "    call putchar\n";
                    
                    label_counter++;
                }
            }
            break;
        }
        
        case NodeType::IfStatement: {
            auto ifNode = std::static_pointer_cast<IfNode>(node);
            int if_label = label_counter++;
            
            // Generate condition code - jumps to else (or end if no else) when false
            if (ifNode->elseBranch) {
                // Jump to else branch when condition is false
                generate_condition(ifNode->condition, asm_code, var_offsets, if_label, "if_else");
            } else {
                // Jump to end when condition is false
                generate_condition(ifNode->condition, asm_code, var_offsets, if_label, "if");
            }
            
            // Generate then branch
            generate_node(ifNode->thenBranch, asm_code, var_offsets, stack_offset, label_counter);
            
            // If there's an else branch, jump over it after then branch
            if (ifNode->elseBranch) {
                asm_code << "    jmp .if_end_" << if_label << "\n";
                asm_code << ".if_else_end_" << if_label << ":\n";
                
                // Generate else branch
                generate_node(ifNode->elseBranch, asm_code, var_offsets, stack_offset, label_counter);
            }
            
            asm_code << ".if_end_" << if_label << ":\n";
            break;
        }
        
        case NodeType::WhileLoop: {
            auto whileNode = std::static_pointer_cast<WhileNode>(node);
            int while_label = label_counter++;
            
            asm_code << ".while_start_" << while_label << ":\n";
            
            // Generate condition code
            generate_condition(whileNode->condition, asm_code, var_offsets, while_label, "while");
            
            // Generate body
            generate_node(whileNode->body, asm_code, var_offsets, stack_offset, label_counter);
            
            asm_code << "    jmp .while_start_" << while_label << "\n";
            asm_code << ".while_end_" << while_label << ":\n";
            break;
        }
        
        case NodeType::ForLoop: {
            auto forNode = std::static_pointer_cast<ForNode>(node);
            int for_label = label_counter++;
            
            // Generate initialization
            generate_node(forNode->init, asm_code, var_offsets, stack_offset, label_counter);
            
            asm_code << ".for_start_" << for_label << ":\n";
            
            // Generate condition
            generate_condition(forNode->condition, asm_code, var_offsets, for_label, "for");
            
            // Generate body
            generate_node(forNode->body, asm_code, var_offsets, stack_offset, label_counter);
            
            // Generate increment
            generate_node(forNode->increment, asm_code, var_offsets, stack_offset, label_counter);
            
            asm_code << "    jmp .for_start_" << for_label << "\n";
            asm_code << ".for_end_" << for_label << ":\n";
            break;
        }
        
        case NodeType::VectorAlloc: {
            // Vector allocation is handled in generate_expression
            // This case shouldn't be reached in generate_node
            break;
        }
        
        case NodeType::ArrayAccess: {
            // Array access is handled in generate_expression
            // This case shouldn't be reached in generate_node
            break;
        }
        
        case NodeType::FunctionDecl: {
            // function name begin params end front body back
            auto funcNode = std::static_pointer_cast<FunctionDeclNode>(node);
            
            asm_code << "\n; Function: " << funcNode->functionName << "\n";
            asm_code << funcNode->functionName << ":\n";
            asm_code << "    push rbp\n";
            asm_code << "    mov rbp, rsp\n";
            asm_code << "    sub rsp, 256\n";  // Local variable space
            
            // Create local variable map for parameters
            // Windows x64: first 4 params in rcx, rdx, r8, r9
            std::map<std::string, int> func_vars;
            int param_offset = 0;
            for (size_t i = 0; i < funcNode->parameters.size(); i++) {
                param_offset += 8;
                func_vars[funcNode->parameters[i]] = param_offset;
                
                // Store parameter on stack
                if (i == 0) {
                    asm_code << "    mov [rbp-" << param_offset << "], rcx\n";
                } else if (i == 1) {
                    asm_code << "    mov [rbp-" << param_offset << "], rdx\n";
                } else if (i == 2) {
                    asm_code << "    mov [rbp-" << param_offset << "], r8\n";
                } else if (i == 3) {
                    asm_code << "    mov [rbp-" << param_offset << "], r9\n";
                } else {
                    // Additional parameters on stack (passed by caller)
                    asm_code << "    mov rax, [rbp+" << (16 + (i-4)*8) << "]\n";
                    asm_code << "    mov [rbp-" << param_offset << "], rax\n";
                }
            }
            
            int func_stack_offset = param_offset;
            int func_label_counter = 0;
            
            // Generate function body
            generate_node(funcNode->body, asm_code, func_vars, func_stack_offset, func_label_counter);
            
            // Default return (return 0)
            asm_code << "    xor rax, rax\n";
            asm_code << "    add rsp, 256\n";
            asm_code << "    pop rbp\n";
            asm_code << "    ret\n\n";
            break;
        }
        
        case NodeType::Return: {
            // mcs expression.
            auto returnNode = std::static_pointer_cast<ReturnNode>(node);
            
            // Evaluate return expression
            generate_expression(returnNode->returnValue, asm_code, var_offsets);
            
            // Return value is in rax, clean up and return
            asm_code << "    add rsp, 256\n";
            asm_code << "    pop rbp\n";
            asm_code << "    ret\n";
            break;
        }
        
        case NodeType::FunctionCall: {
            // Function calls are handled in generate_expression
            // But if it's a statement (not used in expression), handle it here
            generate_expression(node, asm_code, var_offsets);
            break;
        }
        
        default:
            break;
    }
}

// Generate code for an expression (returns result in rax)
void generate_expression(std::shared_ptr<ASTNode> node,
                        std::stringstream& asm_code,
                        std::map<std::string, int>& var_offsets) {
    if (!node) return;
    
    switch (node->type) {
        case NodeType::Literal: {
            // Convert base-17 to decimal
            std::string val = node->value.value();
            int decimal = base17_to_decimal(val);
            asm_code << "    mov rax, " << decimal << "\n";
            break;
        }
        
        case NodeType::StringLiteral: {
            // Load address of string literal into rax
            std::string str = node->value.value();
            int str_id = string_literals[str];
            asm_code << "    lea rax, [rel str_" << str_id << "]\n";
            break;
        }
        
        case NodeType::Identifier: {
            std::string var_name = node->value.value();
            if (var_offsets.find(var_name) == var_offsets.end()) {
                throw std::runtime_error("Variable '" + var_name + "' not defined");
            }
            asm_code << "    mov rax, [rbp-" << var_offsets[var_name] << "]\n";
            break;
        }
        
        case NodeType::BinaryOp: {
            auto binOp = std::static_pointer_cast<BinaryOpNode>(node);
            
            // Check if this is a string concatenation operation
            if (binOp->op == TokenType::_durham && 
                (is_string_expression(binOp->left, string_variables) || 
                 is_string_expression(binOp->right, string_variables))) {
                // String concatenation
                generate_string_concat(binOp->left, binOp->right, asm_code, 
                                     var_offsets, string_variables, string_literals);
            } else {
                // Numeric operation
                // Generate left operand
                generate_expression(binOp->left, asm_code, var_offsets);
                asm_code << "    push rax\n";
                
                // Generate right operand
                generate_expression(binOp->right, asm_code, var_offsets);
                asm_code << "    mov rbx, rax\n";
                asm_code << "    pop rax\n";
                
                // Perform operation
                switch (binOp->op) {
                    case TokenType::_durham:  // +
                        asm_code << "    add rax, rbx\n";
                        break;
                    case TokenType::_newcastle:  // -
                        asm_code << "    sub rax, rbx\n";
                        break;
                    case TokenType::_york:  // *
                        asm_code << "    imul rax, rbx\n";
                        break;
                    case TokenType::_edinburgh:  // /
                        asm_code << "    xor rdx, rdx\n";
                        asm_code << "    div rbx\n";
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        
        case NodeType::VectorAlloc: {
            // new college begin SIZE end
            auto vecNode = std::static_pointer_cast<VectorAllocNode>(node);
            
            // Evaluate size expression
            generate_expression(vecNode->size, asm_code, var_offsets);
            
            // Allocate from heap: size * 8 bytes (each element is 64-bit)
            asm_code << "    imul rax, 8\n";  // Convert to bytes
            asm_code << "    mov rbx, [rel heap_ptr]\n";  // Get current heap pointer
            asm_code << "    mov rcx, rbx\n";  // Save pointer to return
            asm_code << "    add rbx, rax\n";  // Advance heap pointer
            asm_code << "    mov [rel heap_ptr], rbx\n";  // Store new heap pointer
            asm_code << "    mov rax, rcx\n";  // Return allocated pointer
            break;
        }
        
        case NodeType::ArrayAccess: {
            // array at index
            auto accessNode = std::static_pointer_cast<ArrayAccessNode>(node);
            std::string array_name = accessNode->arrayName;
            
            if (var_offsets.find(array_name) == var_offsets.end()) {
                throw std::runtime_error("Array '" + array_name + "' not defined");
            }
            
            // Get the array pointer (stored in variable)
            asm_code << "    mov rbx, [rbp-" << var_offsets[array_name] << "]\n";
            
            // Evaluate index expression
            generate_expression(accessNode->index, asm_code, var_offsets);
            
            // Calculate offset: index * 8 (each element is 8 bytes)
            asm_code << "    imul rax, 8\n";
            
            // Load array[index] into rax
            asm_code << "    add rbx, rax\n";
            asm_code << "    mov rax, [rbx]\n";
            break;
        }
        
        case NodeType::FunctionCall: {
            // func begin arg1 and arg2 end
            std::string func_name = node->value.value();
            
            asm_code << "    ; Call function " << func_name << "\n";
            
            // Windows x64 calling convention: rcx, rdx, r8, r9, then stack
            // Save caller-saved registers if needed
            asm_code << "    push rcx\n";
            asm_code << "    push rdx\n";
            asm_code << "    push r8\n";
            asm_code << "    push r9\n";
            
            // Align stack to 16 bytes (Windows x64 requirement)
            asm_code << "    sub rsp, 32\n";  // Shadow space
            
            // Evaluate arguments and place them in registers/stack
            for (size_t i = 0; i < node->children.size(); i++) {
                generate_expression(node->children[i], asm_code, var_offsets);
                
                if (i == 0) {
                    asm_code << "    mov rcx, rax\n";
                } else if (i == 1) {
                    asm_code << "    mov rdx, rax\n";
                } else if (i == 2) {
                    asm_code << "    mov r8, rax\n";
                } else if (i == 3) {
                    asm_code << "    mov r9, rax\n";
                } else {
                    // Push additional args onto stack (right-to-left)
                    asm_code << "    push rax\n";
                }
            }
            
            // Call the function
            asm_code << "    call " << func_name << "\n";
            
            // Clean up stack
            asm_code << "    add rsp, 32\n";
            
            // Restore registers
            asm_code << "    pop r9\n";
            asm_code << "    pop r8\n";
            asm_code << "    pop rdx\n";
            asm_code << "    pop rcx\n";
            
            // Result is now in rax
            break;
        }
        
        default:
            break;
    }
}

// Generate code for a condition
void generate_condition(std::shared_ptr<ASTNode> node,
                       std::stringstream& asm_code,
                       std::map<std::string, int>& var_offsets,
                       int label,
                       const std::string& label_prefix) {
    if (!node) return;
    
    if (node->type == NodeType::BinaryOp) {
        auto binOp = std::static_pointer_cast<BinaryOpNode>(node);
        
        // Handle logical operators (or/and)
        if (binOp->op == TokenType::_or) {
            // Short-circuit OR: if left is true, skip evaluation of right
            // We need to invert the logic: jump to end only if BOTH conditions are false
            auto leftBin = std::static_pointer_cast<BinaryOpNode>(binOp->left);
            auto rightBin = std::static_pointer_cast<BinaryOpNode>(binOp->right);
            
            // Create a temporary label for "condition satisfied"
            int temp_label = label + 2000;
            
            // Evaluate left condition - if TRUE, skip the jump to end
            generate_expression(leftBin->left, asm_code, var_offsets);
            asm_code << "    push rax\n";
            generate_expression(leftBin->right, asm_code, var_offsets);
            asm_code << "    mov rbx, rax\n";
            asm_code << "    pop rax\n";
            asm_code << "    cmp rax, rbx\n";
            
            // Jump past end-jump if left condition is TRUE
            switch (leftBin->op) {
                case TokenType::_lesser:
                    asm_code << "    jl .or_satisfied_" << temp_label << "\n";
                    break;
                case TokenType::_greater:
                    asm_code << "    jg .or_satisfied_" << temp_label << "\n";
                    break;
                case TokenType::_equals:
                    asm_code << "    je .or_satisfied_" << temp_label << "\n";
                    break;
                case TokenType::_not_equals:
                    asm_code << "    jne .or_satisfied_" << temp_label << "\n";
                    break;
                default:
                    break;
            }
            
            // Left was false, evaluate right condition
            generate_expression(rightBin->left, asm_code, var_offsets);
            asm_code << "    push rax\n";
            generate_expression(rightBin->right, asm_code, var_offsets);
            asm_code << "    mov rbx, rax\n";
            asm_code << "    pop rax\n";
            asm_code << "    cmp rax, rbx\n";
            
            // Jump to end if right condition is also FALSE
            switch (rightBin->op) {
                case TokenType::_lesser:
                    asm_code << "    jge ." << label_prefix << "_end_" << label << "\n";
                    break;
                case TokenType::_greater:
                    asm_code << "    jle ." << label_prefix << "_end_" << label << "\n";
                    break;
                case TokenType::_equals:
                    asm_code << "    jne ." << label_prefix << "_end_" << label << "\n";
                    break;
                case TokenType::_not_equals:
                    asm_code << "    je ." << label_prefix << "_end_" << label << "\n";
                    break;
                default:
                    break;
            }
            
            asm_code << ".or_satisfied_" << temp_label << ":\n";
            return;
        }
        
        if (binOp->op == TokenType::_and) {
            // Short-circuit AND: if left is false, skip to end
            int temp_label = label + 1000;
            generate_expression(binOp->left, asm_code, var_offsets);
            asm_code << "    test rax, rax\n";
            asm_code << "    jz .and_false_" << temp_label << "\n";
            generate_condition(binOp->right, asm_code, var_offsets, label, label_prefix);
            asm_code << ".and_false_" << temp_label << ":\n";
            return;
        }
        
        // Handle comparison operators
        generate_expression(binOp->left, asm_code, var_offsets);
        asm_code << "    push rax\n";
        generate_expression(binOp->right, asm_code, var_offsets);
        asm_code << "    mov rbx, rax\n";
        asm_code << "    pop rax\n";
        asm_code << "    cmp rax, rbx\n";
        
        // Jump to end if condition is FALSE
        switch (binOp->op) {
            case TokenType::_lesser:
                asm_code << "    jge ." << label_prefix << "_end_" << label << "\n";
                break;
            case TokenType::_greater:
                asm_code << "    jle ." << label_prefix << "_end_" << label << "\n";
                break;
            case TokenType::_equals:
                asm_code << "    jne ." << label_prefix << "_end_" << label << "\n";
                break;
            case TokenType::_not_equals:
                asm_code << "    je ." << label_prefix << "_end_" << label << "\n";
                break;
            default:
                break;
        }
    }
}

// Helper function declarations (add these to gen_asm.h or at the top)
void generate_node(std::shared_ptr<ASTNode> node, 
                   std::stringstream& asm_code,
                   std::map<std::string, int>& var_offsets,
                   int& stack_offset,
                   int& label_counter);

void generate_expression(std::shared_ptr<ASTNode> node,
                        std::stringstream& asm_code,
                        std::map<std::string, int>& var_offsets);

void generate_condition(std::shared_ptr<ASTNode> node,
                       std::stringstream& asm_code,
                       std::map<std::string, int>& var_offsets,
                       int label);

