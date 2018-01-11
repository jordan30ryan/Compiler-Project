enum TokenType 
{
    PERIOD, SEMICOLON, L_PAREN, R_PAREN, COMMA, IN, OUT, INOUT, L_BRACKET, R_BRACKET, COLON, PROGRAM, IS, BEGIN, END, GLOBAL, PROCEDURE, CHAR, INTEGER, FLOAT, BOOL, IF, THEN, ELSE, FOR, RETURN, ASSIGNMENT, AND, OR, NOT, PLUS, MINUS, LT, GT, LT_EQ, GT_EQ, EQUAL, NOTEQUAL, MULTIPLICATION, DIVISION, TRUE, FALSE, IDENTIFIER, FILE_END, STRING
};

enum CharClass 
{
    LETTER, DIGIT, OPERATOR, SYMBOLS 
};

struct Token
{
    TokenType type;
    union
    {
        char* string_value;
        int int_value;
        int double_value;
    } val;
};

