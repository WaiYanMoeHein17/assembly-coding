#ifndef GEN_ASM_H
#define GEN_ASM_H

#include "tokenizer.h"
#include "parser.h"
#include <string>
#include <vector>
#include <memory>

// Original function (keep for backward compatibility)
std::string generate(std::vector<Token> tokens);

// New AST-based generator
std::string generate_assembly_from_ast(std::shared_ptr<ASTNode> ast);

// Helper for base-17 conversion
int base17_to_decimal(const std::string& base17_str);

#endif