#pragma once
#include <map>

#define MAX_STRING_LEN 256


// Reserved words begin with RS_
enum TokenType 
{
    PERIOD, SEMICOLON, L_PAREN, R_PAREN, COMMA, L_BRACKET, R_BRACKET, COLON, AND, OR, PLUS, MINUS, LT, GT, LT_EQ, GT_EQ, ASSIGNMENT, EQUALS, NOTEQUAL, MULTIPLICATION, DIVISION, FILE_END, STRING, CHAR, INTEGER, FLOAT, BOOL, IDENTIFIER, UNKNOWN,
    RS_IN, RS_OUT, RS_INOUT, RS_PROGRAM, RS_IS, RS_BEGIN, RS_END, RS_GLOBAL, RS_PROCEDURE, RS_STRING, RS_CHAR, RS_INTEGER, RS_FLOAT, RS_BOOL, RS_IF, RS_THEN, RS_ELSE, RS_FOR, RS_RETURN, RS_TRUE, RS_FALSE, RS_NOT
};

enum CharClass 
{
    SYMBOL=0, LETTER, DIGIT, WHITESPACE
};

struct Token
{
    TokenType type;
    union
    {
        char string_value[MAX_STRING_LEN];
        char char_value;
        int int_value;
        double double_value;
    } val;
    int line;
};

struct ReservedWordRecord
{
    ReservedWordRecord(TokenType type, const char* value) : type(type), value(value) {}
    TokenType type;
    const char* value;
};


