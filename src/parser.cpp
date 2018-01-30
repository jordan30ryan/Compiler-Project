#include "parser.h"

// DEBUG


#include <iostream>
const char* TokenTypeStrings[] = 
{
    "PERIOD", "SEMICOLON", "L_PAREN", "R_PAREN", "COMMA", "L_BRACKET", "R_BRACKET", "COLON", "AND", "OR", "PLUS", "MINUS", "LT", "GT", "LT_EQ", "GT_EQ", "ASSIGNMENT", "EQUALS", "NOTEQUAL", "MULTIPLICATION", "DIVISION", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT", "IDENTIFIER", "UNKNOWN"
};

// Gets a vector of tokens from the scanner
void scanner_debug(Scanner* scanner) 
{
    Token token;
    while ((token = scanner->getToken()).type != TokenType::FILE_END)
    {
        std::cout << token.line << '\t';
        std::cout << TokenTypeStrings[token.type] << '\t';
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::STRING)
        {
            std::cout << '"' << token.val.string_value << '"';
        }
        if (token.type == TokenType::CHAR)
        {
            std::cout << '\'' << token.val.char_value << '\'';
        }
        if (token.type == TokenType::INTEGER)
        {
            std::cout << token.val.int_value;
        }
        if (token.type == TokenType::FLOAT)
        {
            std::cout << token.val.double_value;
        }
        std::cout << std::endl;
    }
}

// END DEBUG



Parser::Parser(Scanner* scan) : scanner(scan) { }

void Parser::parse() 
{
    //scanner_debug(scanner);
    Token token = scanner->getToken();
    if (token.type == TokenType::RS_PROGRAM) 
    {
        program();
    }
}

void Parser::program()
{
    Token id = scanner->getToken();
    if (id.type == TokenType::IDENTIFIER)
    {
        char* program_name = id.val.string_value;
        std::cout << "Prog. name is " << program_name << '\n';
    }
}
void Parser::program_header()
{
}

void Parser::program_body()
{
}

void Parser::identifier()
{
}

void Parser::declaration()
{
}


void Parser::proc_declaration()
{
}

void Parser::proc_header()
{
}

void Parser::proc_body()
{
}

void Parser::parameter_list()
{
}

void Parser::parameter()
{
}


void Parser::var_declaration()
{
}

void Parser::type_mark()
{
}

void Parser::lower_bound()
{
}

void Parser::upper_bound()
{
}


void Parser::statement()
{
}

void Parser::assignment_statement()
{
}

void Parser::if_statement()
{
}

void Parser::loop_statement()
{
}

void Parser::return_statement()
{
}

void Parser::proc_call()
{
}


void Parser::argument_list()
{
}

void Parser::destination()
{
}


void Parser::expression()
{
}

void Parser::arith_op()
{
}

void Parser::relation()
{
}

void Parser::term()
{
}

void Parser::factor()
{
}

void Parser::name()
{
}

void Parser::number()
{
}

void Parser::string_literal()
{
}

void Parser::char_literal()
{
}



