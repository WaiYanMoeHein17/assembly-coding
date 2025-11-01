#include <iostream> 
#include <fstream>
#include <sstream>
#include <string> 
#include <vector> 
#include <optional>
#include <cctype>
#include <map>
#include "main.hpp"

//using namespace std;

std::optional<char> college_to_digit(const std::string& college) {
    static const std::map<std::string,char> lookup = {
        {"butler", '0'},
        {"chads", '1'},
        {"marys", '2'},
        {"collingwood", '3'},
        {"johns", '4'},
        {"castle", '5'},
        {"cuths", '6'},
        {"trevs", '7'},
        {"aidans", '8'},
        {"snow", '9'}
    };
    /*
    if (lookup(college) != nullptr) {
        return lookup[college]; 
    } else {
        return std::nullopt;     
    }
    */
    auto it = lookup.find(college);
    if (it != lookup.end()) return it->second;
    return std::nullopt;
}

std::vector<Token> tokenize(const std::string& text) {
    std::vector<Token> tokens; 
    std::string buffer; 

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i]; 
        if (std::isspace(c)) {
            continue;
        }
        else if(c == ';') {
            tokens.push_back({TokenType::semi}); 
            continue; 
        }
        else if(c == '=') {
            tokens.push_back({TokenType::equals});
            continue;
        }
        else if (c == '(') {
            tokens.push_back({TokenType::open_paren});
            continue;
        }
        else if (c == ')') {
            tokens.push_back({TokenType::close_paren});
            continue;
        }
        else if (c == '{') {
            tokens.push_back({TokenType::open_brace});
            continue;
        }
        else if (c == '}') {
            tokens.push_back({TokenType::close_brace});
            continue;
        }
        else if (c == ',') {
            tokens.push_back({TokenType::comma}); 
            continue;
        }
        else if (c == '.') {
            tokens.push_back({TokenType::dot}); 
            continue;
        }
        else if (std::isalpha(c)) {
            buffer.clear(); 
            std::string number_string; 

            while (i < text.length() && std::isalpha(text[i])) {
                buffer += text[i]; 
                i++;
            }

            auto digit = college_to_digit(buffer); 
            if (digit.has_value()) {
                number_string += digit.value(); 

                while (i < text.length() && text[i] == '_') {
                    i++;
                    buffer.clear();
                    while (i < text.length() && std::isalpha(text[i])) {
                        buffer += text[i]; 
                        i++;
                    }
                    digit = college_to_digit(buffer);
                    if (digit.has_value()) {
                        number_string += digit.value(); 
                    } else {
                        std::cerr << "Error: Unknown college name '" << buffer << "'" << std::endl; 
                        break; 
                    }
                }
                i--; 
                tokens.push_back({TokenType::int_lit, number_string});
            } else {
                i--;
                // Check for keywords and operators
                if (buffer == "tlc") {
                    tokens.push_back({TokenType::_tlc});
                } else if (buffer == "durham") {
                    tokens.push_back({TokenType::_durham});  // +
                } else if (buffer == "hat") {
                    tokens.push_back({TokenType::_hat});     // -
                } else if (buffer == "ustinov") {
                    tokens.push_back({TokenType::_ustinov}); // *
                } else if (buffer == "steven") {
                    tokens.push_back({TokenType::_steven});  // /
                } else if (buffer == "grey") {
                    tokens.push_back({TokenType::_grey});    // %
                } else if (buffer == "durhack") {
                    tokens.push_back({TokenType::_durhack});
                } else {
                    tokens.push_back({TokenType::identifier, buffer});
                }
            } 
        }
    }
    return tokens; 
}

void ask_user_info(struct User& user) {
    std::string college;
    std::string course; 
    char gender; 
    std::cout << "Which college are you from: "; 
    std::cin >> college; 
    std::cout << "Which course do you take: "; 
    std::cin >> course; 
    std::cout << "Gender? "; 
    std::cin >> gender; 
    user.college = college;
    user.course = course; 
    user.gender = gender; 
}

void make_jokes(User user) {

}

std::string generate_assembly(const std::vector<Token>& tokens) {
    std::stringstream assembly_code; 
    
    std::map<std::string, int> variables;  // For vectors (stores offset to size)
    std::map<std::string, int> scalars;    // For scalar variables (stores value)
    int stack_offset = 0;

    assembly_code << "section .data\n";
    assembly_code << "    digit db '0', 10\n";
    assembly_code << "    array times 1000 dq 0\n\n";
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
                    numbers.push_back(std::stoi(tokens[i].value.value()));
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
        // Handle scalar assignment: name = number;
        else if (i + 2 < tokens.size() &&
                 tokens[i].type == TokenType::identifier &&
                 tokens[i + 1].type == TokenType::equals &&
                 tokens[i + 2].type == TokenType::int_lit) {
            
            std::string var_name = tokens[i].value.value();
            int value = std::stoi(tokens[i + 2].value.value());
            
            scalars[var_name] = value;
            
            assembly_code << "    ; Scalar variable '" << var_name << "' = " << value << "\n\n";
            
            i += 2; // Skip name, =, number (semicolon handled by main loop)
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
                int value = scalars[var_name];
                
                assembly_code << "    ; Print scalar '" << var_name << "'\n";
                std::string value_str = std::to_string(value);
                for (char digit_char : value_str) {
                    assembly_code << "    mov rcx, " << static_cast<int>(digit_char) << "\n";
                    assembly_code << "    call putchar\n";
                }
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
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
                
                int left = std::stoi(expr_tokens[0].value.value());
                int right = std::stoi(expr_tokens[2].value.value());
                int result = 0;
                
                if (expr_tokens[1].type == TokenType::_durham) {
                    result = left + right;
                } else if (expr_tokens[1].type == TokenType::_hat) {
                    result = left - right;
                } else if (expr_tokens[1].type == TokenType::_ustinov) {
                    result = left * right;
                } else if (expr_tokens[1].type == TokenType::_steven) {
                    result = left / right;
                } else if (expr_tokens[1].type == TokenType::_grey) {
                    result = left % right;
                }
                
                std::string result_str = std::to_string(result);
                for (char digit_char : result_str) {
                    assembly_code << "    mov rcx, " << static_cast<int>(digit_char) << "\n";
                    assembly_code << "    call putchar\n";
                }
                assembly_code << "    mov rcx, 10\n";
                assembly_code << "    call putchar\n\n";
                
            } else if (expr_tokens.size() == 1 && expr_tokens[0].type == TokenType::int_lit) {
                std::string number = expr_tokens[0].value.value();
                for (char digit_char : number) {
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

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect Usage" << std::endl; 
        std::cerr << "Correct Usage: durham <input.dur>" << std::endl;  // Changed
        return EXIT_FAILURE;  // Fixed: should be FAILURE not SUCCESS
    }
    //std::cout << argv[1] << std::endl; 
    //std::cout<< "Hello World" << std::endl;

    std::ifstream input(argv[1]); // ifstream ONLY input
    
    if (!input.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl; 
        return EXIT_FAILURE; 
    }

    std::stringstream buffer; 
    buffer << input.rdbuf(); 
    std::string contents = buffer.str(); 

    input.close(); // always close after opening file

    std::vector<Token> tokens = tokenize(contents); 
    // debug
    std::string assembly_code = generate_assembly(tokens);

    // Write assembly
    std::ofstream output("output.asm");
    if (output.is_open()) {
        output << assembly_code;
        output.close();
        //std::cout << "Generated output.asm" << std::endl;
    } else {
        std::cerr << "Error: Could not write output.asm" << std::endl;
        return EXIT_FAILURE;
    }

    // Automatically assemble and link
    //std::cout << "Assembling..." << std::endl;
    int result = system("nasm -f win64 output.asm -o output.obj");
    if (result != 0) {
        std::cerr << "Assembly failed" << std::endl;
        return EXIT_FAILURE;
    }

    //std::cout << "Linking..." << std::endl;
    result = system("gcc output.obj -o output.exe");
    if (result != 0) {
        std::cerr << "Linking failed" << std::endl;
        return EXIT_FAILURE;
    }

    //std::cout << "Running program..." << std::endl;
    result = system("output.exe");
    
    return result;
}