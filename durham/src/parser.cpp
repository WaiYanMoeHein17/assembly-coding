#include "main.h"
#include "parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) 
    : tokens(tokens), current(0) {}

// Helper methods
Token Parser::peek(int offset) {
    if (current + offset >= tokens.size()) {
        return tokens.back(); // Return last token if out of bounds
    }
    return tokens[current + offset];
}

Token Parser::advance() {
    if (!at_end()) current++;
    return tokens[current - 1];
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (at_end()) return false;
    return peek().type == type;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(message);
}

bool Parser::at_end() {
    return current >= tokens.size();
}

// Main parse method
std::shared_ptr<ASTNode> Parser::parse() {
    return parseProgram();
}

// Parse entire program
std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto program = std::make_shared<ASTNode>(NodeType::Program);
    
    while (!at_end()) {
        auto stmt = parseStatement();
        if (stmt) {
            program->children.push_back(stmt);
        }
    }
    
    return program;
}

// Parse a statement
std::shared_ptr<ASTNode> Parser::parseStatement() {
    // Skip semicolons
    if (match(TokenType::semi)) {
        return nullptr;
    }
    
    // Function declaration
    if (check(TokenType::_function)) {
        return parseFunctionDecl();
    }
    
    // Return statement
    if (check(TokenType::_mcs)) {
        return parseReturn();
    }
    
    // If statement
    if (check(TokenType::_if)) {
        return parseIfStatement();
    }
    
    // While loop
    if (check(TokenType::_while)) {
        return parseWhileLoop();
    }
    
    // For loop
    if (check(TokenType::_for)) {
        return parseForLoop();
    }
    
    // Print statement
    if (check(TokenType::_tlc)) {
        return parsePrint();
    }
    
    // Typed variable declaration: text name is ... OR number x is ...
    if (check(TokenType::_text) || check(TokenType::_number)) {
        return parseTypedDeclaration();
    }
    
    // Assignment
    if (check(TokenType::identifier)) {
        return parseAssignment();
    }
    
    advance(); // Skip unknown token
    return nullptr;
}

// Parse assignment: x is expr. OR array at index is expr. OR function call: func begin args end.
std::shared_ptr<ASTNode> Parser::parseAssignment() {
    Token name = consume(TokenType::identifier, "Expected variable name");
    
    // Check if it's a function call: name begin args end
    if (check(TokenType::open_paren)) {
        auto funcCall = parseFunctionCall(name.value.value());
        consume(TokenType::semi, "Expected '.' after function call");
        return funcCall;
    }
    
    // Check if it's array element assignment: array at index is value
    if (check(TokenType::_at)) {
        consume(TokenType::_at, "Expected 'at'");
        auto arrayAccess = std::make_shared<ArrayAccessNode>(name.value.value());
        arrayAccess->index = parseExpression();
        
        consume(TokenType::equals, "Expected 'is' after array index");
        
        // Create an assignment node with the array access on the left
        auto assignment = std::make_shared<AssignmentNode>(name.value.value());
        assignment->left = arrayAccess;  // Array access
        assignment->right = parseExpression();  // Value to assign
        
        consume(TokenType::semi, "Expected '.' after expression");
        return assignment;
    }
    
    // Regular variable assignment
    consume(TokenType::equals, "Expected 'is' after variable name");
    
    auto assignment = std::make_shared<AssignmentNode>(name.value.value());
    assignment->right = parseExpression();
    
    consume(TokenType::semi, "Expected '.' after expression");
    
    return assignment;
}

// Parse typed variable declaration: text name is "value". OR number x is butler.
std::shared_ptr<ASTNode> Parser::parseTypedDeclaration() {
    // Consume type keyword (text or number)
    Token typeToken = advance();
    std::string varType = (typeToken.type == TokenType::_text) ? "text" : "number";
    
    // Get variable name
    Token name = consume(TokenType::identifier, "Expected variable name after type");
    
    // Consume 'is'
    consume(TokenType::equals, "Expected 'is' after variable name");
    
    // Parse the value expression
    auto assignment = std::make_shared<AssignmentNode>(name.value.value(), varType);
    assignment->right = parseExpression();
    
    // Type checking: verify the expression matches the declared type
    if (varType == "text") {
        // For text variables, we need to check if the expression is a string literal or string variable
        if (assignment->right->type != NodeType::StringLiteral && 
            assignment->right->type != NodeType::Identifier &&
            assignment->right->type != NodeType::BinaryOp) {
            throw std::runtime_error("Type error: text variable '" + name.value.value() + 
                                   "' must be assigned a string value");
        }
    } else if (varType == "number") {
        // For number variables, check if it's a numeric literal or numeric expression
        if (assignment->right->type == NodeType::StringLiteral) {
            throw std::runtime_error("Type error: number variable '" + name.value.value() + 
                                   "' cannot be assigned a string value");
        }
    }
    
    consume(TokenType::semi, "Expected '.' after expression");
    
    return assignment;
}

// Parse expression: term ((+ | -) term)*
std::shared_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();
    
    while (match(TokenType::_durham) || match(TokenType::_newcastle)) {
        TokenType op = tokens[current - 1].type;
        auto node = std::make_shared<BinaryOpNode>(op);
        node->left = left;
        node->right = parseTerm();
        left = node;
    }
    
    return left;
}

// Parse term: factor ((* | /) factor)*
std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    
    while (match(TokenType::_york) || match(TokenType::_edinburgh)) {
        TokenType op = tokens[current - 1].type;
        auto node = std::make_shared<BinaryOpNode>(op);
        node->left = left;
        node->right = parseFactor();
        left = node;
    }
    
    return left;
}

// Parse factor: primary or (expression) or string
std::shared_ptr<ASTNode> Parser::parseFactor() {
    if (match(TokenType::open_paren)) {
        // Check if it's a string literal: begin "text" end
        if (check(TokenType::quotations)) {
            consume(TokenType::quotations, "Expected string");
            auto stringNode = std::make_shared<ASTNode>(NodeType::StringLiteral);
            stringNode->value = tokens[current - 1].value.value();
            consume(TokenType::close_paren, "Expected 'end' after string");
            return stringNode;
        }
        
        // Otherwise it's a parenthesized expression
        auto expr = parseExpression();
        consume(TokenType::close_paren, "Expected 'end' after expression");
        return expr;
    }
    
    return parsePrimary();
}

// Parse primary: number or identifier
std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (match(TokenType::int_lit)) {
        return std::make_shared<LiteralNode>(tokens[current - 1].value.value());
    }
    
    // Vector allocation: new college begin SIZE end
    if (match(TokenType::_new)) {
        return parseVectorAlloc();
    }
    
    if (match(TokenType::identifier)) {
        std::string name = tokens[current - 1].value.value();
        
        // Check for function call: identifier begin args end
        if (check(TokenType::open_paren)) {
            return parseFunctionCall(name);
        }
        
        // Check for array access: identifier at index
        if (check(TokenType::_at)) {
            return parseArrayAccess(name);
        }
        
        // Regular identifier
        auto node = std::make_shared<ASTNode>(NodeType::Identifier, name);
        return node;
    }
    
    throw std::runtime_error("Expected expression");
}

// Parse condition: expr (< | > | == | !=) expr (or/and expr)*
std::shared_ptr<ASTNode> Parser::parseCondition() {
    auto left = parseExpression();
    
    // Comparison operators
    if (match(TokenType::_lesser) || match(TokenType::_greater) || 
        match(TokenType::_equals) || match(TokenType::_not_equals)) {
        TokenType op = tokens[current - 1].type;
        auto node = std::make_shared<BinaryOpNode>(op);
        node->left = left;
        node->right = parseExpression();
        left = node;
    }
    
    // Logical operators (or/and)
    while (match(TokenType::_or) || match(TokenType::_and)) {
        TokenType op = tokens[current - 1].type;
        auto node = std::make_shared<BinaryOpNode>(op);
        node->left = left;
        node->right = parseCondition();
        left = node;
    }
    
    return left;
}

// Parse if: if begin condition end front body back
std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    consume(TokenType::_if, "Expected 'if'");
    consume(TokenType::open_paren, "Expected 'begin' after 'if'");
    
    auto ifNode = std::make_shared<IfNode>();
    ifNode->condition = parseCondition();
    
    consume(TokenType::close_paren, "Expected 'end' after condition");
    consume(TokenType::open_brace, "Expected 'front' after condition");
    
    ifNode->thenBranch = parseBlock();
    
    consume(TokenType::close_brace, "Expected 'back' after if body");
    
    // Check for else clause
    if (current < tokens.size() && peek().type == TokenType::_else) {
        consume(TokenType::_else, "Expected 'else'");
        consume(TokenType::open_brace, "Expected 'front' after 'else'");
        
        ifNode->elseBranch = parseBlock();
        
        consume(TokenType::close_brace, "Expected 'back' after else body");
    }
    
    return ifNode;
}

// Parse while: while begin condition end front body back
std::shared_ptr<ASTNode> Parser::parseWhileLoop() {
    consume(TokenType::_while, "Expected 'while'");
    consume(TokenType::open_paren, "Expected 'begin' after 'while'");
    
    auto whileNode = std::make_shared<WhileNode>();
    whileNode->condition = parseCondition();
    
    consume(TokenType::close_paren, "Expected 'end' after condition");
    consume(TokenType::open_brace, "Expected 'front' after condition");
    
    whileNode->body = parseBlock();
    
    consume(TokenType::close_brace, "Expected 'back' after while body");
    
    return whileNode;
}

// Parse for: for begin init . condition . increment end front body back
std::shared_ptr<ASTNode> Parser::parseForLoop() {
    consume(TokenType::_for, "Expected 'for'");
    consume(TokenType::open_paren, "Expected 'begin' after 'for'");
    
    auto forNode = std::make_shared<ForNode>();
    
    // Initialization
    forNode->init = parseAssignment();
    
    // Condition
    forNode->condition = parseCondition();
    consume(TokenType::semi, "Expected '.' after condition");
    
    // Increment (parse assignment without consuming semicolon)
    Token name = consume(TokenType::identifier, "Expected variable name");
    consume(TokenType::equals, "Expected 'is' after variable name");
    auto assignment = std::make_shared<AssignmentNode>(name.value.value());
    assignment->right = parseExpression();
    forNode->increment = assignment;
    // Note: No semicolon consumed here - 'end' comes directly after increment
    
    consume(TokenType::close_paren, "Expected 'end' after for header");
    consume(TokenType::open_brace, "Expected 'front' after for header");
    
    forNode->body = parseBlock();
    
    consume(TokenType::close_brace, "Expected 'back' after for body");
    
    return forNode;
}

// Parse block of statements
std::shared_ptr<ASTNode> Parser::parseBlock() {
    auto block = std::make_shared<ASTNode>(NodeType::Block);
    
    while (!check(TokenType::close_brace) && !at_end()) {
        auto stmt = parseStatement();
        if (stmt) {
            block->children.push_back(stmt);
        }
    }
    
    return block;
}

// Parse print: tlc begin expr end.
std::shared_ptr<ASTNode> Parser::parsePrint() {
    consume(TokenType::_tlc, "Expected 'tlc'");
    consume(TokenType::open_paren, "Expected 'begin' after 'tlc'");
    
    auto printNode = std::make_shared<ASTNode>(NodeType::Print);
    
    // Check if it's a string literal: tlc begin "string" end.
    if (check(TokenType::quotations)) {
        Token strToken = advance();
        printNode->value = strToken.value;  // Store the string
    } else {
        // Regular expression: tlc begin expr end.
        printNode->left = parseExpression();
    }
    
    consume(TokenType::close_paren, "Expected 'end' after expression");
    consume(TokenType::semi, "Expected '.' after print statement");
    
    return printNode;
}

// Parse vector allocation: new college begin SIZE end
std::shared_ptr<ASTNode> Parser::parseVectorAlloc() {
    consume(TokenType::_college, "Expected 'college' after 'new'");
    consume(TokenType::open_paren, "Expected 'begin' after 'college'");
    
    auto vectorNode = std::make_shared<VectorAllocNode>();
    vectorNode->size = parseExpression();
    
    consume(TokenType::close_paren, "Expected 'end' after size");
    
    return vectorNode;
}

// Parse array access: array at index
std::shared_ptr<ASTNode> Parser::parseArrayAccess(const std::string& arrayName) {
    consume(TokenType::_at, "Expected 'at'");
    
    auto accessNode = std::make_shared<ArrayAccessNode>(arrayName);
    accessNode->index = parseExpression();
    
    return accessNode;
}

// Parse function declaration: function name begin param1 and param2 end front body back
std::shared_ptr<ASTNode> Parser::parseFunctionDecl() {
    consume(TokenType::_function, "Expected 'function'");
    
    Token nameToken = consume(TokenType::identifier, "Expected function name");
    auto funcNode = std::make_shared<FunctionDeclNode>(nameToken.value.value());
    
    consume(TokenType::open_paren, "Expected 'begin' after function name");
    
    // Parse parameters (separated by 'and')
    if (!check(TokenType::close_paren)) {
        do {
            Token param = consume(TokenType::identifier, "Expected parameter name");
            funcNode->parameters.push_back(param.value.value());
            
            if (match(TokenType::_and)) {
                continue; // More parameters
            } else {
                break;
            }
        } while (true);
    }
    
    consume(TokenType::close_paren, "Expected 'end' after parameters");
    consume(TokenType::open_brace, "Expected 'front' before function body");
    
    // Parse function body
    funcNode->body = parseBlock();
    
    consume(TokenType::close_brace, "Expected 'back' after function body");
    
    return funcNode;
}

// Parse return statement: mcs expression.
std::shared_ptr<ASTNode> Parser::parseReturn() {
    consume(TokenType::_mcs, "Expected 'mcs'");
    
    auto returnNode = std::make_shared<ReturnNode>();
    returnNode->returnValue = parseExpression();
    
    consume(TokenType::semi, "Expected '.' after return value");
    
    return returnNode;
}

// Parse function call: name begin arg1 and arg2 end
std::shared_ptr<ASTNode> Parser::parseFunctionCall(const std::string& functionName) {
    auto callNode = std::make_shared<ASTNode>(NodeType::FunctionCall, functionName);
    
    consume(TokenType::open_paren, "Expected 'begin' for function call");
    
    // Parse arguments (separated by 'and')
    if (!check(TokenType::close_paren)) {
        do {
            auto arg = parseExpression();
            callNode->children.push_back(arg);
            
            if (match(TokenType::_and)) {
                continue; // More arguments
            } else {
                break;
            }
        } while (true);
    }
    
    consume(TokenType::close_paren, "Expected 'end' after function arguments");
    
    return callNode;
}






