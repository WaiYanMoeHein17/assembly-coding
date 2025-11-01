#ifndef MAIN_HPP
#define MAIN_HPP

#include <string> 
#include <optional> 
#include <vector> 

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

    // Arithmetic operators
    _durham,        // +
    _hat,           // -
    _ustinov,       // *
    _steven,        // /
    _grey,          // %

    // I/O / utility
    _tlc,           //.print()
    _newcastle,     //.return()
    _durhack,       //.average()
    _durhackten,    //.max()
    _durhackone,    //.min()

    // Vector/string-like operations
    _billyb,        //.sort()
    _mcs,           //.reverse()
    _south,         //.remove()
    _hild,          //.move() 
    _bede,          //.find()
    _van,           //.begin()
    _mildert,       //.end()
    _whinney_hill,  //.up()
    _gilesgate,     // ?

    // For and If
    _cardiac_hill,  // for
    _cs_degree,     // if
    _mountjoy,      // else

    // Logical Operators
    _framwellgate,   // AND
    _claypath,       // OR
    _botanic_gardens,// NOT
    // Comparisons
    _elvet,         //==
    _maiden_castle, //!=
    _bailey,        // > 
    _hill,          // < 

    // Punctuation and others
    semi,           // ; 
    open_paren,     // (
    close_paren,    // )
    open_brace,     // {
    close_brace,    // }
    equals,         // =
    int_lit, 
    comma,          //,
    dot,            //.
    identifier      // variable names
};

struct Token {
    TokenType type; 
    std::optional<std::string> value; 
}; 

struct User {
    std::optional<std::string> name; 
    std::optional<std::string> college; 
    std::optional<std::string> course;
    std::optional<char> gender; 
    std::optional<int> age; 
}; 

struct Settings {
    bool jokes = true; 
    bool auto_correct = true;
    std::string college_one;
}; 

std::optional<char> college_to_digit(const std::string& college);
std::vector<Token> tokenize(const std::string& text);
std::string generate_assembly(const std::vector<Token>& tokens);
void ask_user_info(); 
void make_joke(User user); // make jokes depending on the user info 
void autocorrect(int argc, int **argv); // automatically corrects code in .dur file

#endif //MAIN_HPP