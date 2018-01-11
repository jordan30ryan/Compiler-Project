#include <map>

enum TokenType 
{
    PERIOD, SEMICOLON, L_PAREN, R_PAREN, COMMA, IN, OUT, INOUT, L_BRACKET, R_BRACKET, COLON, PROGRAM, IS, BEGIN, END, GLOBAL, PROCEDURE, CHAR, INTEGER, FLOAT, BOOL, IF, THEN, ELSE, FOR, RETURN, ASSIGNMENT, AND, OR, NOT, PLUS, MINUS, LT, GT, LT_EQ, GT_EQ, EQUAL, NOTEQUAL, MULTIPLICATION, DIVISION, TRUE, FALSE, IDENTIFIER, FILE_END, STRING
};


enum CharClass 
{
    UNDEFINED=0, LETTER, DIGIT, OPERATOR, SYMBOLS, WHITESPACE
};

struct Token
{
    TokenType type;
    union
    {
        //TODO: how to not limit strings to 256 chars
        char string_value[256];
        char char_value;
        int int_value;
        int double_value;
    } val;
};




// DEBUG

static std::map<TokenType, char const*> debug_typemap;

//debug_typemap.insert(std::pair<TokenType, char const*>(PERIOD, "PERIOD"));
static void init_debug()
{
    debug_typemap.insert(std::pair<TokenType, char const*>(PERIOD, "PERIOD"));
    debug_typemap.insert(std::pair<TokenType, char const*>(SEMICOLON, "SEMICOLON"));
    debug_typemap.insert(std::pair<TokenType, char const*>(L_PAREN, "L_PAREN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(R_PAREN, "R_PAREN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(COMMA, "COMMA"));
    debug_typemap.insert(std::pair<TokenType, char const*>(IN, "IN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(OUT, "OUT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(INOUT, "INOUT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(L_BRACKET, "L_BRACKET"));
    debug_typemap.insert(std::pair<TokenType, char const*>(R_BRACKET, "R_BRACKET"));
    debug_typemap.insert(std::pair<TokenType, char const*>(COLON, "COLON"));
    debug_typemap.insert(std::pair<TokenType, char const*>(PROGRAM, "PROGRAM"));
    debug_typemap.insert(std::pair<TokenType, char const*>(IS, "IS"));
    debug_typemap.insert(std::pair<TokenType, char const*>(BEGIN, "BEGIN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(END, "END"));
    debug_typemap.insert(std::pair<TokenType, char const*>(GLOBAL, "GLOBAL"));
    debug_typemap.insert(std::pair<TokenType, char const*>(PROCEDURE, "PROCEDURE"));
    debug_typemap.insert(std::pair<TokenType, char const*>(CHAR, "CHAR"));
    debug_typemap.insert(std::pair<TokenType, char const*>(INTEGER, "INTEGER"));
    debug_typemap.insert(std::pair<TokenType, char const*>(FLOAT, "FLOAT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(BOOL, "BOOL"));
    debug_typemap.insert(std::pair<TokenType, char const*>(IF, "IF"));
    debug_typemap.insert(std::pair<TokenType, char const*>(THEN, "THEN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(ELSE, "ELSE"));
    debug_typemap.insert(std::pair<TokenType, char const*>(FOR, "FOR"));
    debug_typemap.insert(std::pair<TokenType, char const*>(RETURN, "RETURN"));
    debug_typemap.insert(std::pair<TokenType, char const*>(ASSIGNMENT, "ASSIGNMENT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(AND, "AND"));
    debug_typemap.insert(std::pair<TokenType, char const*>(OR, "OR"));
    debug_typemap.insert(std::pair<TokenType, char const*>(NOT, "NOT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(PLUS, "PLUS"));
    debug_typemap.insert(std::pair<TokenType, char const*>(MINUS, "MINUS"));
    debug_typemap.insert(std::pair<TokenType, char const*>(LT, "LT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(GT, "GT"));
    debug_typemap.insert(std::pair<TokenType, char const*>(LT_EQ, "LT_EQ"));
    debug_typemap.insert(std::pair<TokenType, char const*>(GT_EQ, "GT_EQ"));
    debug_typemap.insert(std::pair<TokenType, char const*>(EQUAL, "EQUAL"));
    debug_typemap.insert(std::pair<TokenType, char const*>(NOTEQUAL, "NOTEQUAL"));
    debug_typemap.insert(std::pair<TokenType, char const*>(MULTIPLICATION, "MULTIPLICATION"));
    debug_typemap.insert(std::pair<TokenType, char const*>(DIVISION, "DIVISION"));
    debug_typemap.insert(std::pair<TokenType, char const*>(TRUE, "TRUE"));
    debug_typemap.insert(std::pair<TokenType, char const*>(FALSE, "FALSE"));
    debug_typemap.insert(std::pair<TokenType, char const*>(IDENTIFIER, "IDENTIFIER"));
    debug_typemap.insert(std::pair<TokenType, char const*>(FILE_END, "FILE_END"));
    debug_typemap.insert(std::pair<TokenType, char const*>(STRING, "STRING"));
}

// END DEBUG

