#include "tokenizer.h"

// Constructor
Tokenizer::Tokenizer(const std::string& source) : src(source), index(0) {}

// Destructor
Tokenizer::~Tokenizer() {}

std::optional<char> Tokenizer::peek(int offset) {
    if (index + offset >= src.length()) {
        return {}; 
    }
    return src.at(index + offset);
}

char Tokenizer::consume() {
    return src.at(index++); 
}

std::optional<char> college_to_digit(const std::string& college) {
    static const std::map<std::string, char> lookup = {
        {"butler", '0'},
        {"chads", '1'},
        {"marys", '2'},
        {"collingwood", '3'},
        {"johns", '4'},
        {"castle", '5'},
        {"cuths", '6'},
        {"trevs", '7'},
        {"aidans", '8'},
        {"snow", '9'},
        {"grey", 'A'},           // 10
        {"stephenson", 'B'},     // 11
        {"hatfield", 'C'},       // 12
        {"hild_bede", 'D'},      // 13
        {"south", 'E'},          // 14
        {"van_mildert", 'F'},    // 15
        {"ustinov", 'G'}         // 16
    };
    
    auto it = lookup.find(college);
    if (it != lookup.end()) return it->second;
    return std::nullopt;
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    
    while (peek().has_value()) {
        // Skip whitespace
        if (std::isspace(peek().value())) {
            consume();
            continue;
        }
        // Single character tokens
        if (peek().value() == '.') {
            consume();
            tokens.push_back({TokenType::semi});  // Changed: '.' is semicolon equivalent
            continue;
        }
        
        // String literals with quotation marks
        if (peek().value() == '"') {
            consume(); // consume opening quote
            std::string str_value;
            
            while (peek().has_value() && peek().value() != '"') {
                str_value += consume();
            }
            
            if (peek().has_value() && peek().value() == '"') {
                consume(); // consume closing quote
                tokens.push_back({TokenType::quotations, str_value});
            } else {
                std::cerr << "Error: Unterminated string literal" << std::endl;
            }
            continue;
        }
        
        // Alphabetic tokens (keywords, identifiers, college names)
        if (std::isalpha(peek().value())) {
            std::string buffer;
            
            // Consume all alphabetic characters
            while (peek().has_value() && std::isalpha(peek().value())) {
                buffer += consume();
            }
            
            // Check if it's a college name (digit)
            auto digit = college_to_digit(buffer);
            if (digit.has_value()) {
                std::string number_string;
                number_string += digit.value();
                
                // Handle multi-digit numbers with ','
                while (peek().has_value() && peek().value() == ',') {
                    consume(); // consume comma
                    
                    buffer.clear();
                    while (peek().has_value() && std::isalpha(peek().value())) {
                        buffer += consume();
                    }
                    
                    digit = college_to_digit(buffer);
                    if (digit.has_value()) {
                        number_string += digit.value();
                    } else {
                        std::cerr << "Error: Unknown college name '" << buffer << "'" << std::endl;
                        break;
                    }
                }
                
                tokens.push_back({TokenType::int_lit, number_string});
            } 
            // Check for keywords
            else if (buffer == "tlc") {
                tokens.push_back({TokenType::_tlc});
            } else if (buffer == "mcs") {
                tokens.push_back({TokenType::_mcs});
            } else if (buffer == "for") {
                tokens.push_back({TokenType::_for});
            } else if (buffer == "if") {
                tokens.push_back({TokenType::_if});
            } else if (buffer == "while") {
                tokens.push_back({TokenType::_while});
            // Vector/Array keywords
            } else if (buffer == "new") {
                tokens.push_back({TokenType::_new});
            } else if (buffer == "college") {
                tokens.push_back({TokenType::_college});
            } else if (buffer == "at") {
                tokens.push_back({TokenType::_at});
            // Arithmetic operators
            } else if (buffer == "durham") {
                tokens.push_back({TokenType::_durham});
            } else if (buffer == "newcastle") {
                tokens.push_back({TokenType::_newcastle});
            } else if (buffer == "york") {
                tokens.push_back({TokenType::_york});
            } else if (buffer == "edinburgh") {
                tokens.push_back({TokenType::_edinburgh});
            // Logical operators
            } else if (buffer == "and") {
                tokens.push_back({TokenType::_and});
            } else if (buffer == "or") {
                tokens.push_back({TokenType::_or});
            } else if (buffer == "not") {
                tokens.push_back({TokenType::_not});
            // Comparison operators
            } else if (buffer == "greater") {
                tokens.push_back({TokenType::_greater});
            } else if (buffer == "lesser") {
                tokens.push_back({TokenType::_lesser});
            } else if (buffer == "equals") {
                tokens.push_back({TokenType::_equals});
            } else if (buffer == "not") {
                tokens.push_back({TokenType::_not});
            } else if (buffer == "is") {
                tokens.push_back({TokenType::equals});  // "is" keyword for assignment
            } else if (buffer == "begin") {
                tokens.push_back({TokenType::open_paren}); 
            } else if (buffer == "end") {
                tokens.push_back({TokenType::close_paren}); 
            } else if (buffer == "front") {
                tokens.push_back({TokenType::open_brace}); 
            } else if (buffer == "back") {
                tokens.push_back({TokenType::close_brace}); 
            } else {
                // It's an identifier (variable name)
                tokens.push_back({TokenType::identifier, buffer});
            }
            
            continue;
        }
        
        // Unknown character - skip it
        std::cerr << "Warning: Unknown character '" << peek().value() << "'" << std::endl;
        consume();
    }
    
    // Post-processing: combine consecutive tokens
    std::vector<Token> processed_tokens;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (i + 1 < tokens.size() && 
            tokens[i].type == TokenType::_not && 
            tokens[i + 1].type == TokenType::_equals) {
            // Combine into not_equals
            processed_tokens.push_back({TokenType::_not_equals});
            i++; // Skip the next token since we combined them
        } else {
            processed_tokens.push_back(tokens[i]);
        }
    }
    
    return processed_tokens;
}

