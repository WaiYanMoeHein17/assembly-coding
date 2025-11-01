#ifndef PARSER_H
#define PARSER_H

#include <memory> 
#include <vector>
#include "main.h"
#include "tokenizer.h"


//using namespace std; 

struct ASTNode;

enum class NodeType {
    Program, 
    Statement, 
    Assignment, 
    BinaryOp, 
    UnaryOp, 
    Literal, 
    Identifier, 
    FunctionCall, 
    IfStatement, 
    ForLoop,
    WhileLoop, 
    Block, 
    Condition, 
    Print,
    VectorAlloc,    // new college begin SIZE end
    ArrayAccess     // ARRAY at INDEX
}; 

struct ASTNode {
    NodeType type; 
    std::optional<std::string> value; 
    std::shared_ptr<ASTNode> left; 
    std::shared_ptr<ASTNode> right; 
    std::vector<std::shared_ptr<ASTNode>> children; 

    ASTNode(NodeType t) : type(t), left(nullptr), right(nullptr) {}
    ASTNode(NodeType t, const std::string& val) : type(t), value(val), left(nullptr), right(nullptr) {}
}; 

struct LiteralNode : public ASTNode {
    std::string literalValue;
    
    LiteralNode(const std::string& val) 
        : ASTNode(NodeType::Literal, val), literalValue(val) {}
};

struct BinaryOpNode : public ASTNode {
    TokenType op;
    
    BinaryOpNode(TokenType operation) 
        : ASTNode(NodeType::BinaryOp), op(operation) {}
};

struct AssignmentNode : public ASTNode {
    std::string varName;
    
    AssignmentNode(const std::string& name) 
        : ASTNode(NodeType::Assignment, name), varName(name) {}
};

struct IfNode : public ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> thenBranch;
    std::shared_ptr<ASTNode> elseBranch;
    
    IfNode() : ASTNode(NodeType::IfStatement) {}
};

struct WhileNode : public ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> body;
    
    WhileNode() : ASTNode(NodeType::WhileLoop) {}
};

struct ForNode : public ASTNode {
    std::shared_ptr<ASTNode> init; 
    std::shared_ptr<ASTNode> condition; 
    std::shared_ptr<ASTNode> increment; 
    std::shared_ptr<ASTNode> body;
    
    ForNode() : ASTNode(NodeType::ForLoop) {}
}; 

struct VectorAllocNode : public ASTNode {
    std::shared_ptr<ASTNode> size;  // Size expression
    
    VectorAllocNode() : ASTNode(NodeType::VectorAlloc) {}
};

struct ArrayAccessNode : public ASTNode {
    std::string arrayName;
    std::shared_ptr<ASTNode> index;  // Index expression
    
    ArrayAccessNode(const std::string& name) 
        : ASTNode(NodeType::ArrayAccess, name), arrayName(name) {}
};

class Parser {
    private:
        std::vector<Token> tokens; 
        size_t current; 

        Token peek(int offset = 0); 
        Token advance(); 
        Token consume(TokenType type, const std::string& message); 
        bool match(TokenType type); 
        bool check(TokenType type);
        bool at_end(); 

        std::shared_ptr<ASTNode> parseProgram();
        std::shared_ptr<ASTNode> parseStatement();
        std::shared_ptr<ASTNode> parseAssignment();
        std::shared_ptr<ASTNode> parseExpression();
        std::shared_ptr<ASTNode> parseComparison();
        std::shared_ptr<ASTNode> parseTerm();
        std::shared_ptr<ASTNode> parseFactor();
        std::shared_ptr<ASTNode> parsePrimary();
        std::shared_ptr<ASTNode> parseIfStatement();
        std::shared_ptr<ASTNode> parseWhileLoop();
        std::shared_ptr<ASTNode> parseForLoop();
        std::shared_ptr<ASTNode> parseBlock();
        std::shared_ptr<ASTNode> parsePrint();
        std::shared_ptr<ASTNode> parseCondition();
        std::shared_ptr<ASTNode> parseVectorAlloc();
        std::shared_ptr<ASTNode> parseArrayAccess(const std::string& arrayName);
    public: 
        Parser(const std::vector<Token>& tokens);
        std::shared_ptr<ASTNode> parse(); 

}; 

#endif //PARSER_H