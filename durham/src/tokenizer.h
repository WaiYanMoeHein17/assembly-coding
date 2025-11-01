#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <optional> 
#include <vector> 
#include <map>
#include <cctype>
#include <iostream>

enum class TokenType {
    // Numeric colleges -> digits 0-9
    _butler,        //0
    _chads,         //1
    _marys,         //2
    _collingwood,   //3
    _johns,         //4
    _castle,        //5
    _cuths,         //6
    _trevs,         //7
    _aidans,        //8
    _snow,          //9
    _grey,          //10 
    _stephenson,    //11
    _hatfield,      //12 
    _hildbede,     //13
    _south,         //14
    _vanmildert,   //15
    _ustinov,       //16

    // Arithmetic operators
    _durham,        // +
    _newcastle,     // -
    _york,          // *
    _edinburgh,     // / 

    // I/O / utility
    _tlc,           // print
    _mcs,           // return

    _for,           //for
    _if,            //if 
    _while,         //while

    // Vector/Array operations
    _new,           // new
    _college,       // college (array/vector type)
    _at,            // at (array access)

    // Logical Operators
    _and,           // AND
    _or,            // OR
    _not,           // NOT

    _greater,       // >  
    _lesser,        // <
    _equals,        // ==
    _not_equals,    // !=          

    // Punctuation and others
    quotations,     // " "
    semi,           // .
    open_paren,     // begin
    close_paren,    // end
    open_brace,     // front
    close_brace,    // back
    equals,         // is
    int_lit, 
    comma,          // and
    dot,            // '
    identifier      // variable names
};

struct Token {
    TokenType type; 
    std::optional<std::string> value; 
}; 

class Tokenizer {
    public:
        
        Tokenizer(const std::string& source); // constructor
        ~Tokenizer(); // destructor
        std::vector<Token> tokenize(); 

    private: 
        
        int index; 
        std::string src; 
        std::optional<char> peek(int offset=0);
        char consume();
        
};

std::optional<char> college_to_digit(const std::string& college);



#endif //TOKENIZER_H