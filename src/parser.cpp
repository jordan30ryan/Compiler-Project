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
    std::cout << "Got: " << TokenTypeStrings[curr_token.type] << '\n';
    return curr_token;
}

Token Parser::getToken(TokenType type)
{
    getToken();
    if (curr_token.type != type)
    {
        // TODO Error handling
        std::cerr << "Bad token (expected " << TokenTypeStrings[type] << ", found " << TokenTypeStrings[curr_token.type] << ").\n";
    }
    return curr_token;
}

void Parser::parse() 
{
    //scanner_debug(scanner);

    program();
    getToken(TokenType::FILE_END);
}

void Parser::program()
{
    std::cout << "program" << '\n';
    // NOTE: First token of a certain group is always consumed
    //  before entering that group (to allow for 
    getToken(TokenType::RS_PROGRAM); 
    program_header(); 
    program_body(); 
    getToken(TokenType::PERIOD); 
}

void Parser::program_header()
{
    std::cout << "program header" << '\n';
    getToken(TokenType::IDENTIFIER); 
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        char* program_name = curr_token.val.string_value;
        std::cout << "Prog. name is " << program_name << '\n';
    }
    getToken(TokenType::RS_IS); 
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
            getToken(TokenType::RS_PROGRAM); // RS_PROGRAM
            return;
        }
        if (declarations) declaration();
        else statement();
        getToken(TokenType::SEMICOLON); 
    }
}

void Parser::identifier()
{
    std::cout << "identifier" << '\n';
}

void Parser::declaration()
{
    std::cout << "declaration" << '\n';
    bool global;
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
    getToken(TokenType::IDENTIFIER);
    getToken(TokenType::L_PAREN);
    //TODO
    getToken(); // <R_PAREN|param_list>
    if (curr_token.type == TokenType::R_PAREN)
    {
        return;
    }
    else 
    {
        parameter_list(); 
    }
    getToken(TokenType::R_PAREN);
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
            getToken(TokenType::RS_PROCEDURE); 
            return;
        }
        if (declarations) declaration();
        else statement();
        getToken(TokenType::SEMICOLON); 
    }
}

void Parser::parameter_list()
{
    std::cout << "param list" << '\n';
}

void Parser::parameter()
{
    std::cout << "param" << '\n';
}


void Parser::var_declaration()
{
    std::cout << "var decl" << '\n';
    TokenType typemark = curr_token.type;
    char* id = getToken(TokenType::IDENTIFIER).val.string_value;
    // TODO: lower/upper bound stuff
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



