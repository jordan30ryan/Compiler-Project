#pragma once
#include <unordered_map>
#include <string>

// Reserved words begin with RS_
// Note: <number> in the spec is expanded to INTEGER and FLOAT here.
//  This should simplify typechecking and hopefully won't introduce any issues
enum TokenType 
{
    PERIOD, SEMICOLON, L_PAREN, R_PAREN, COMMA, L_BRACKET, R_BRACKET, COLON, AND, OR, PLUS, MINUS, LT, GT, LT_EQ, GT_EQ, ASSIGNMENT, EQUALS, NOTEQUAL, MULTIPLICATION, DIVISION, FILE_END, STRING, CHAR, INTEGER, FLOAT, BOOL, IDENTIFIER, UNKNOWN,
    RS_IN, RS_OUT, RS_INOUT, RS_PROGRAM, RS_IS, RS_BEGIN, RS_END, RS_GLOBAL, RS_PROCEDURE, RS_STRING, RS_CHAR, RS_INTEGER, RS_FLOAT, RS_BOOL, RS_IF, RS_THEN, RS_ELSE, RS_FOR, RS_RETURN, RS_TRUE, RS_FALSE, RS_NOT
};

struct Value
{
    std::string string_value;
    char char_value;
    int int_value;
    double float_value;
};

struct Token
{
    TokenType type;
    Value val;
    int line;
};

enum SymbolType
{
    S_UNDEFINED, S_STRING, S_CHAR, S_INTEGER, S_FLOAT, S_BOOL, S_PROCEDURE
};

struct SymTableEntry
{
    // For RS tokens
    SymTableEntry(TokenType t) : type(t) {}
    // For Identifiers
    SymTableEntry() : type(IDENTIFIER) {}

    TokenType type; 

    // if type==IDENTIFIER, stores identifier type (i.e. Variable type or Procedure)
    SymbolType sym_type = S_UNDEFINED;
    // If type==IDENTIFER && sym_type==S_PROCEDURE, stores the proc local symbol table
    //std::unordered_map<std::string, SymTableEntry> local_symbols;
};

