#include "parser.h"

// DEBUG


#include <iostream>
const char* TokenTypeStrings[] = 
//{
//    "PERIOD", "SEMICOLON", "L_PAREN", "R_PAREN", "COMMA", "L_BRACKET", "R_BRACKET", "COLON", "AND", "OR", "PLUS", "MINUS", "LT", "GT", "LT_EQ", "GT_EQ", "ASSIGNMENT", "EQUALS", "NOTEQUAL", "MULTIPLICATION", "DIVISION", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT", "IDENTIFIER", "UNKNOWN"
//};
{
"PERIOD", "SEMICOLON", "L_PAREN", "R_PAREN", "COMMA", "L_BRACKET", "R_BRACKET", "COLON", "AND", "OR", "PLUS", "MINUS", "LT", "GT", "LT_EQ", "GT_EQ", "ASSIGNMENT", "EQUALS", "NOTEQUAL", "MULTIPLICATION", "DIVISION", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "IDENTIFIER", "UNKNOWN",
"RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT"
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

        // curr_token should be SEMICOLON
    }
}

void Parser::declaration()
{
    std::cout << "declaration" << '\n';
    bool global = false;
    if (curr_token.type == TokenType::RS_GLOBAL)
    {
        global = true;
        getToken(); 
    }
    // curr_token := <RS_PROCEDURE|typemark>
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
    getToken(); // SEMICOLON
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
            return;
        }
        
        if (declarations) declaration();
        else statement();

        // curr_token should be SEMICOLON
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
        else return; // R_PAREN
    }
}

void Parser::parameter()
{
    std::cout << "param" << '\n';
    var_declaration();
    // curr_token should be <IN|OUT|INOUT>
    TokenType param_type = curr_token.type;
    std::cout << "Param type: " << TokenTypeStrings[param_type] << '\n';
}


void Parser::var_declaration()
{
    std::cout << "var decl" << '\n';
    type_mark();
    getToken(); // IDENTIFIER
    char* id = curr_token.val.string_value;
    std::cout << "Identifier: " << id << '\n';
    getToken(); // L_BRACKET|<SEMICOLON|IN|OUT|INOUT>
    if (curr_token.type == TokenType::L_BRACKET)
    {
        lower_bound();
        getToken(); // COLON
        upper_bound();
        getToken(); // R_BRACKET
        getToken(); // SEMICOLON|in|out|inout
    }
    else return;
    // At return, curr_token should be <SEMICOLON|IN|OUT|INOUT>
}

void Parser::type_mark()
{
    std::cout << "type_mark" << '\n';
    TokenType typemark = curr_token.type;
}

void Parser::lower_bound()
{
    std::cout << "lower_bound" << '\n';
    getToken(); // NUMBER
}

void Parser::upper_bound()
{
    std::cout << "upper_bound" << '\n';
    getToken(); // NUMBER
}


bool Parser::statement()
{
    std::cout << "stmnt" << '\n';
    // curr_token is statement[0]
    bool valid = false;
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        const char* identifier = curr_token.val.string_value;
        getToken(); // ASSIGNMENT|L_PAREN
        if (curr_token.type == TokenType::ASSIGNMENT)
        {
            valid = assignment_statement();
        }
        else if (curr_token.type == TokenType::L_PAREN)
        {
            valid = proc_call();
        }
        else valid = false; // IDENTIFIER followed by something other 
                            //   than ASSIGNMENT or L_PAREN; error.
    }
    else if (curr_token.type == TokenType::RS_IF)
    {
        valid = if_statement();
    }
    else if (curr_token.type == TokenType::RS_FOR)
    {
        valid = loop_statement();
    }
    else if (curr_token.type == TokenType::RS_RETURN)
    {
        valid = return_statement();
    }
    else valid = false;

    getToken(); // SEMICOLON
    return valid; 
}

bool Parser::assignment_statement()
{
    std::cout << "assignment stmnt" << '\n';
    expression();
    return true;
}

bool Parser::if_statement()
{
    std::cout << "if" << '\n';
    getToken(); // (
    expression();
    //getToken(); // )
    getToken(); // RS_THEN
    while (true)
    {
        getToken(); // statement[0]|ELSE|END
        // TODO: Ensure there's at least one statement
        if (!statement()) break;
        // curr_token = SEMICOLON
    }

    if (curr_token.type == TokenType::RS_ELSE)
    {
        while (true)
        {
            getToken(); // statement[0]|END
            // TODO: Ensure there's at least one statement
            if (!statement()) break;
            // curr_token = SEMICOLON
        }       
    }
    // curr_token should be RS_END
    getToken(); // RS_IF
    return true;
}

bool Parser::loop_statement()
{
    std::cout << "loop" << '\n';
    // curr_token = RS_FOR
    getToken(); // (
    assignment_statement(); 
    getToken(); // ;
    expression();
    getToken(); // )
    while (true)
    {
        getToken(); // statement[0]|END
        if (!statement()) break;
        // curr_token = SEMICOLON
    }
    // curr_token = RS_END
    getToken(); // RS_FOR
    return true;
}

bool Parser::return_statement()
{
    std::cout << "return" << '\n';
    // curr_token = RS_RETURN
    return true;
}

bool Parser::proc_call()
{
    std::cout << "proc call" << '\n';
    // curr_token.type = IDENTIFIER
    //getToken(); // L_PAREN
    argument_list();
    // curr_token.type = R_PAREN
    return true;
}

void Parser::argument_list()
{
    std::cout << "arg list" << '\n';
    getToken(); // expression[0]
    while (true)
    {
        expression();
        getToken(); // maybe ,
        if (curr_token.type == TokenType::COMMA) 
        {
            getToken(); // expression[0]
            continue;
        }
        else return;
    }
}

void Parser::destination()
{
    std::cout << "destination" << '\n';
    // curr_token type = IDENTIFIER
    getToken(); // maybe [
    if (curr_token.type == TokenType::L_BRACKET)
    {
        expression();
        getToken(); // ]
    }
    else return;
}

void Parser::expression()
{
    std::cout << "expr" << '\n';
    getToken(); // expression[0]

    arith_op(); 
    expression_pr();
}

void Parser::expression_pr()
{
    std::cout << "expr prime" << '\n';
    //getToken(); // & or |
    if (curr_token.type == TokenType::AND || curr_token.type == TokenType::OR)
    {
        arith_op();
        expression_pr();
    }
    else return;
}

void Parser::arith_op()
{
    std::cout << "arith op" << '\n';
    if (curr_token.type == TokenType::RS_NOT)
    {
        // TODO: something.
        getToken(); // the real arith_op[0]
    }

    relation();
    arith_op_pr();
}

void Parser::arith_op_pr()
{
    std::cout << "arith op pr" << '\n';
    //getToken(); // + or -
    if (curr_token.type == TokenType::PLUS || curr_token.type == TokenType::MINUS)
    {
        relation(); 
        arith_op_pr();
    }
    else return;
}

void Parser::relation()
{
    std::cout << "relation" << '\n';
    term();
    relation_pr();
}

void Parser::relation_pr()
{
    std::cout << "relation pr" << '\n';
    //getToken(); // Relation
    if ((curr_token.type == TokenType::LT)
        | (curr_token.type == TokenType::GT)
        | (curr_token.type == TokenType::LT_EQ)
        | (curr_token.type == TokenType::GT_EQ)
        | (curr_token.type == TokenType::EQUALS)
        | (curr_token.type == TokenType::NOTEQUAL))
    {
        term();
        relation_pr();
    }
    else return;
}

void Parser::term()
{
    std::cout << "term" << '\n';
    factor();
    term_pr();
}

void Parser::term_pr()
{
    std::cout << "term pr" << '\n';
    getToken(); // * or /
    if ((curr_token.type == TokenType::MULTIPLICATION)
        | (curr_token.type == TokenType::DIVISION))
    {
        factor();
        term_pr();
    }
    else return;
}

void Parser::factor()
{
    std::cout << "factor" << '\n';
    // Token is either (expression), name, number, string, char, true, false
    // TODO
    return;
}

void Parser::name()
{
    std::cout << "name" << '\n';
    // curr_token type IDENTIFIER
    // TODO: Get optional "[" expression "]"
}


