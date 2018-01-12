#include <map>

#define MAX_STRING_LEN 256


// Reserved words begin with RS_
enum TokenType 
{
    PERIOD, SEMICOLON, L_PAREN, R_PAREN, COMMA, L_BRACKET, R_BRACKET, COLON, AND, OR, PLUS, MINUS, LT, GT, LT_EQ, GT_EQ, ASSIGNMENT, EQUALS, NOTEQUAL, MULTIPLICATION, DIVISION, FILE_END, STRING, CHAR, INTEGER, FLOAT, BOOL, RS_IN, RS_OUT, RS_INOUT, RS_PROGRAM, RS_IS, RS_BEGIN, RS_END, RS_GLOBAL, RS_PROCEDURE, RS_CHAR, RS_INTEGER, RS_FLOAT, RS_BOOL, RS_IF, RS_THEN, RS_ELSE, RS_FOR, RS_RETURN, RS_TRUE, RS_FALSE, RS_NOT, IDENTIFIER, UNKNOWN
};

enum CharClass 
{
    DEFAULT=0, LETTER, DIGIT, WHITESPACE, UNDERSCORE
};

struct Token
{
    TokenType type;
    union
    {
        char string_value[MAX_STRING_LEN];
        char char_value;
        int int_value;
        int double_value;
    } val;
};

struct ReservedWordRecord
{
    ReservedWordRecord(TokenType type, const char* value) : type(type), value(value) {}
    TokenType type;
    const char* value;
};

// DEBUG

static std::map<TokenType, const char*> debug_typemap;

static void init_debug()
{
    debug_typemap[TokenType::PERIOD] = "PERIOD";
    debug_typemap[TokenType::SEMICOLON] = "SEMICOLON";
    debug_typemap[TokenType::L_PAREN] = "L_PAREN";
    debug_typemap[TokenType::R_PAREN] = "R_PAREN";
    debug_typemap[TokenType::COMMA] = "COMMA";
    debug_typemap[TokenType::L_BRACKET] = "L_BRACKET";
    debug_typemap[TokenType::R_BRACKET] = "R_BRACKET";
    debug_typemap[TokenType::COLON] = "COLON";
    debug_typemap[TokenType::AND] = " AND";
    debug_typemap[TokenType::OR] = "OR";
    debug_typemap[TokenType::PLUS] = "PLUS";
    debug_typemap[TokenType::MINUS] = "MINUS";
    debug_typemap[TokenType::LT] = "LT";
    debug_typemap[TokenType::GT] = "GT";
    debug_typemap[TokenType::LT_EQ] = "LT_EQ";
    debug_typemap[TokenType::GT_EQ] = "GT_EQ";
    debug_typemap[TokenType::ASSIGNMENT] = "ASSIGNMENT";
    debug_typemap[TokenType::EQUALS] = "EQUALS";
    debug_typemap[TokenType::NOTEQUAL] = "NOTEQUAL";
    debug_typemap[TokenType::MULTIPLICATION] = "MULTIPLICATION";
    debug_typemap[TokenType::DIVISION] = "DIVISION";
    debug_typemap[TokenType::FILE_END] = "FILE_END";
    debug_typemap[TokenType::STRING] = "STRING";
    debug_typemap[TokenType::CHAR] = "CHAR";
    debug_typemap[TokenType::INTEGER] = "INTEGER";
    debug_typemap[TokenType::FLOAT] = "FLOAT";
    debug_typemap[TokenType::BOOL] = "BOOL";
    debug_typemap[TokenType::RS_IN] = "RS_IN";
    debug_typemap[TokenType::RS_OUT] = "RS_OUT";
    debug_typemap[TokenType::RS_INOUT] = "RS_INOUT";
    debug_typemap[TokenType::RS_PROGRAM] = "RS_PROGRAM";
    debug_typemap[TokenType::RS_IS] = "RS_IS";
    debug_typemap[TokenType::RS_BEGIN] = "RS_BEGIN";
    debug_typemap[TokenType::RS_END] = "RS_END";
    debug_typemap[TokenType::RS_GLOBAL] = "RS_GLOBAL";
    debug_typemap[TokenType::RS_PROCEDURE] = "RS_PROCEDURE";
    debug_typemap[TokenType::RS_CHAR] = "RS_CHAR";
    debug_typemap[TokenType::RS_INTEGER] = "RS_INTEGER";
    debug_typemap[TokenType::RS_FLOAT] = "RS_FLOAT";
    debug_typemap[TokenType::RS_BOOL] = "RS_BOOL";
    debug_typemap[TokenType::RS_IF] = "RS_IF";
    debug_typemap[TokenType::RS_THEN] = "RS_THEN";
    debug_typemap[TokenType::RS_ELSE] = "RS_ELSE";
    debug_typemap[TokenType::RS_FOR] = "RS_FOR";
    debug_typemap[TokenType::RS_RETURN] = "RS_RETURN";
    debug_typemap[TokenType::RS_TRUE] = "RS_TRUE";
    debug_typemap[TokenType::RS_FALSE] = "RS_FALSE";
    debug_typemap[TokenType::RS_NOT] = "RS_NOT";
    debug_typemap[TokenType::IDENTIFIER] = "IDENTIFIER";
    debug_typemap[TokenType::UNKNOWN] = "UNKNOWN";
}

// END DEBUG

