#include "parser.h"

// DEBUG


#include <iostream>
const char* TokenTypeStrings[] = 
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



Parser::Parser(Scanner* scan, ErrHandler* handler) 
    : scanner(scan), err_handler(handler) { }

TokenType Parser::token()
{
    if (!curr_token_valid)
    {
        curr_token_valid = true;
        curr_token = scanner->getToken();
        std::cout << "\tGot: " << TokenTypeStrings[curr_token.type] << '\n';
        return curr_token.type;
    }
    else return curr_token.type;
}

Token Parser::advance()
{
    // Mark as consumed
    curr_token_valid = false;
    return curr_token;
}

// Require that the current token is of type t
Token Parser::require(TokenType t)
{
    TokenType type = token();
    advance();
    if (t != type) 
    {
        // Report err
        // TODO clean this up lmao
        err_handler->reportError("Bad type: ");
        err_handler->reportError(TokenTypeStrings[type]);
        err_handler->reportError("Got: ");
        err_handler->reportError(TokenTypeStrings[t]);
    }
    return curr_token;
}


void Parser::parse() 
{
    token();
    program();
    require(TokenType::FILE_END);
}

void Parser::program()
{
    std::cout << "program" << '\n';
    program_header(); 
    program_body(); 
    require(TokenType::PERIOD);
}

void Parser::program_header()
{
    std::cout << "program header" << '\n';
    require(TokenType::RS_PROGRAM);
    require(TokenType::IDENTIFIER);
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        char* program_name = curr_token.val.string_value;
        std::cout << "Prog. name is " << program_name << '\n';
    }
    require(TokenType::RS_IS);
}

void Parser::program_body()
{
    std::cout << "program body" << '\n';
    bool declarations = true;
    while (true)
    {
        token(); // <(BEGIN)|(END)|(first of decl|stmnt)>
        if (curr_token.type == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            token(); // BEGIN|END|(first of stmnt)
        }
        else if (curr_token.type == TokenType::RS_END)
        {
            advance();
            require(TokenType::RS_PROGRAM);
            return;
        }
        
        if (declarations) declaration();
        else statement();

        require(TokenType::SEMICOLON);
    }
}

void Parser::declaration()
{
    std::cout << "declaration" << '\n';
    bool global = false;
    if (curr_token.type == TokenType::RS_GLOBAL)
    {
        global = true;
        advance();
        token(); 
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
    require(TokenType::RS_PROCEDURE);
    require(TokenType::RS_IDENTIFIER);
    require(TokenType::L_PAREN);
    token(); // <R_PAREN|param_list[0]>
    if (curr_token.type == TokenType::R_PAREN)
    {
        advance();
    }
    else 
    {
        parameter_list(); 
    }
}

void Parser::proc_body()
{
    std::cout << "proc body" << '\n';
    bool declarations = true;
    while (true)
    {
        token(); // <(BEGIN)|(END)|(first of decl|stmnt)>
        if (curr_token.type == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            token(); // BEGIN|END|(first of stmnt)
        }
        else if (curr_token.type == TokenType::RS_END)
        {
            advance();
            require(TokenType::RS_PROCEDURE);
            return;
        }
        
        if (declarations) declaration();
        else statement();

        require(TokenType::SEMICOLON);
    }
}

void Parser::parameter_list()
{
    std::cout << "param list" << '\n';
    while (true)
    {
        parameter(); 
        TokenType type = token(); 
        if (type == TokenType::COMMA)
        {
            advance();
            continue;
        }
        else return;
    }
}

void Parser::parameter()
{
    std::cout << "param" << '\n';

    var_declaration();

    token(); // IN|OUT|INOUT
    TokenType param_type = curr_token.type;
    std::cout << "Param type: " << TokenTypeStrings[param_type] << '\n';
    advance();
}


void Parser::var_declaration()
{
    std::cout << "var decl" << '\n';
    type_mark();

    require(TokenType::IDENTIFIER);
    char* id = curr_token.val.string_value;
    std::cout << "Identifier: " << id << '\n';

    TokenType type = token(); 

    // Optionally L_BRACKET, otherwise, don't advance
    if (type == TokenType::L_BRACKET)
    {
        advance();

        lower_bound();
        require(TokenType::COLON);
        upper_bound();
        require(TokenType::R_BRACKET);
    }
    else return;
}

void Parser::type_mark()
{
    std::cout << "type_mark" << '\n';
    TokenType typemark = curr_token.type;
    advance();
}

void Parser::lower_bound()
{
    std::cout << "lower_bound" << '\n';
    require(TokenType::NUMBER);
}

void Parser::upper_bound()
{
    std::cout << "upper_bound" << '\n';
    require(TokenType::NUMBER);
}

bool Parser::statement()
{
    std::cout << "stmnt" << '\n';
    // curr_token is statement[0]
    bool valid = false;
    if (curr_token.type == TokenType::IDENTIFIER)
    {
        const char* identifier = curr_token.val.string_value;
        token(); // ASSIGNMENT|L_PAREN
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

    token(); // SEMICOLON
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
    token(); // (
    expression();
    //token(); // )
    token(); // RS_THEN
    while (true)
    {
        token(); // statement[0]|ELSE|END
        // TODO: Ensure there's at least one statement
        if (!statement()) break;
        // curr_token = SEMICOLON
    }

    if (curr_token.type == TokenType::RS_ELSE)
    {
        while (true)
        {
            token(); // statement[0]|END
            // TODO: Ensure there's at least one statement
            if (!statement()) break;
            // curr_token = SEMICOLON
        }       
    }
    // curr_token should be RS_END
    token(); // RS_IF
    return true;
}

bool Parser::loop_statement()
{
    std::cout << "loop" << '\n';
    // curr_token = RS_FOR
    token(); // (
    assignment_statement(); 
    token(); // ;
    expression();
    token(); // )
    while (true)
    {
        token(); // statement[0]|END
        if (!statement()) break;
        // curr_token = SEMICOLON
    }
    // curr_token = RS_END
    token(); // RS_FOR
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
    //token(); // L_PAREN
    argument_list();
    // curr_token.type = R_PAREN
    return true;
}

void Parser::argument_list()
{
    std::cout << "arg list" << '\n';
    token(); // expression[0]
    while (true)
    {
        expression();
        token(); // maybe ,
        if (curr_token.type == TokenType::COMMA) 
        {
            token(); // expression[0]
            continue;
        }
        else return;
    }
}

void Parser::destination()
{
    std::cout << "destination" << '\n';
    // curr_token type = IDENTIFIER
    token(); // maybe [
    if (curr_token.type == TokenType::L_BRACKET)
    {
        expression();
        token(); // ]
    }
    else return;
}

void Parser::expression()
{
    std::cout << "expr" << '\n';
    token(); // expression[0]

    arith_op(); 
    expression_pr();
}

void Parser::expression_pr()
{
    std::cout << "expr prime" << '\n';
    //token(); // & or |
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
        token(); // the real arith_op[0]
    }

    relation();
    arith_op_pr();
}

void Parser::arith_op_pr()
{
    std::cout << "arith op pr" << '\n';
    //token(); // + or -
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
    //token(); // Relation
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
    token(); // * or /
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


