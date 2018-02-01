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

Token Parser::getToken()
{
    curr_token = scanner->getToken();
    std::cout << "Got: " << TokenTypeStrings[next_token.type] << '\n';
    return curr_token;
}

void Parser::parse() 
{
    //scanner_debug(scanner);
    program();
    getToken(); // FILE_END
}

void Parser::program()
{
    std::cout << "program" << '\n';
    program_header(); 
    program_body(); 
    getToken(); // PERIOD
}

void Parser::program_header()
{
    std::cout << "program header" << '\n';
    getToken(); // RS_PROGRAM
    getToken(); // IDENTIFIER
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        char* program_name = curr_token.val.string_value;
        std::cout << "Prog. name is " << program_name << '\n';
    }
    getToken(); // RS_IS
}

void Parser::program_body()
{
    std::cout << "program body" << '\n';
    bool declarations = true;
    while (true)
    {
        getToken(); // <(BEGIN)|(END)|(first of decl|stmnt)>
        if (curr_token.type == TokenType::RS_BEGIN)
        {
            declarations = false;
            getToken(); // <first of stmnt>
        }
        else if (curr_token.type == TokenType::RS_END)
        {
            getToken(); // RS_PROGRAM
            return;
        }
        if (declarations) declaration();
        else statement();
        getToken(); // SEMICOLON
    }
}

void Parser::identifier()
{
    std::cout << "identifier" << '\n';
}

void Parser::declaration()
{
    std::cout << "declaration" << '\n';
    bool global = false;
    if (curr_token.type == TokenType::RS_GLOBAL)
    {
        global = true;
        getToken(); // <RS_PROCEDURE|typemark>
    }
    if (curr_token.type == TokenType::RS_PROCEDURE)
    {
        proc_declaration();
    }
    else var_declaration(); // typemark - continue into var decl
    // curr_token should be SEMICOLON
}

void Parser::proc_declaration()
{
    std::cout << "proc decl" << '\n';
    proc_header();
    proc_body();
}

void Parser::proc_header()
{
    std::cout << "proc header" << '\n';
    getToken(); // IDENTIFIER
    getToken(); // L_PAREN
    //TODO
    getToken(); // <R_PAREN|param_list[0]>
    if (curr_token.type == TokenType::R_PAREN)
    {
        return;
    }
    else 
    {
        parameter_list(); 
        // curr_Token should be R_PAREN
        return;
    }
}

void Parser::proc_body()
{
    std::cout << "proc body" << '\n';
    bool declarations = true;
    while (true)
    {
        getToken(); // <(BEGIN)|(END)|(first of decl|stmnt)>
        if (curr_token.type == TokenType::RS_BEGIN)
        {
            declarations = false;
            getToken(); // <first of stmnt>
        }
        else if (curr_token.type == TokenType::RS_END)
        {
            getToken(); // RS_PROCEDURE
            break;
        }
        if (declarations) declaration();
        else statement();
        getToken(); // SEMICOLON
    }
}

void Parser::parameter_list()
{
    std::cout << "param list" << '\n';
    // curr_token should be parameter[0]
    while (true)
    {
        parameter(); 
        getToken(); // COMMA|R_PAREN
        if (curr_token.type == TokenType::COMMA)
        {
            getToken(); // parameter[0]
            continue;
        }
        else 
        {
            return;
        }
    }
}

void Parser::parameter()
{
    std::cout << "param" << '\n';
    var_declaration();
    // curr_token should be <IN|OUT|INOUT>
}


void Parser::var_declaration()
{
    std::cout << "var decl" << '\n';
    type_mark();
    getToken(); // IDENTIFIER
    char* id = curr_token.val.string_value;
    getToken(); // L_BRACKET|SEMICOLON
    if (curr_token.type == TokenType::L_BRACKET)
    {
        getToken(); // Lower bound
        lower_bound();
        getToken(); // COLON
        getToken(); // Upper bound
        upper_bound();
        getToken(); // R_BRACKET
    }
    else return;
}

void Parser::type_mark()
{
    std::cout << "type_mark" << '\n';
    TokenType typemark = curr_token.type;
}

void Parser::lower_bound()
{
    std::cout << "lower_bound" << '\n';
}

void Parser::upper_bound()
{
    std::cout << "upper_bound" << '\n';
}


void Parser::statement()
{
    std::cout << "stmnt" << '\n';
    // curr_token is statement[0]
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        // Assignment or proc call
    }
    else if (curr_token.type == TokenType::RS_IF)
    {
        if_statement();
    }
    else if (curr_token.type == TokenType::RS_FOR)
    {
        loop_statement();
    }
    else if (curr_token.type == TokenType::RS_RETURN)
    {
        return_statement();
    }
}

void Parser::assignment_statement()
{
    std::cout << "assignment stmnt" << '\n';
}

void Parser::if_statement()
{
    std::cout << "if" << '\n';
    getToken(); // (
    expression();
    getToken(); // )
    getToken(); // RS_THEN
    // TODO: (statement;)+
    // TODO: else
    getToken(); // RS_END
    getToken(); // RS_IF
}

void Parser::loop_statement()
{
    std::cout << "loop" << '\n';
}

void Parser::return_statement()
{
    std::cout << "return" << '\n';
    // curr_token = RS_RETURN
}

void Parser::proc_call()
{
    std::cout << "proc call" << '\n';
}

void Parser::argument_list()
{
    std::cout << "arg list" << '\n';
}

void Parser::destination()
{
    std::cout << "destination" << '\n';
}

void Parser::expression()
{
    std::cout << "expr" << '\n';
}

void Parser::arith_op()
{
    std::cout << "arith op" << '\n';
}

void Parser::relation()
{
    std::cout << "relation" << '\n';
}

void Parser::term()
{
    std::cout << "term" << '\n';
}

void Parser::factor()
{
    std::cout << "factor" << '\n';
}

void Parser::name()
{
    std::cout << "name" << '\n';
}

void Parser::number()
{
    std::cout << "number" << '\n';
}

void Parser::string_literal()
{
    std::cout << "string literal" << '\n';
}

void Parser::char_literal()
{
    std::cout << "char literal" << '\n';
}

