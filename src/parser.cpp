#include "parser.h"

// DEBUG


#include <iostream>
const char* TokenTypeStrings[] = 
{
"PERIOD", "SEMICOLON", "L_PAREN", "R_PAREN", "COMMA", "L_BRACKET", "R_BRACKET", "COLON", "AND", "OR", "PLUS", "MINUS", "LT", "GT", "LT_EQ", "GT_EQ", "ASSIGNMENT", "EQUALS", "NOTEQUAL", "MULTIPLICATION", "DIVISION", "FILE_END", "STRING", "CHAR", "NUMBER", "BOOL", "IDENTIFIER", "UNKNOWN",
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
        if (token.type == TokenType::NUMBER)
        {
            std::cout << token.val.double_value;
        }
/*
        if (token.type == TokenType::FLOAT)
        {
            std::cout << token.val.double_value;
        }
*/
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
    // Return the current token to be used before getting next token 
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
        err_handler->reportError("Expected: ");
        err_handler->reportError(TokenTypeStrings[t]);
    }
    return curr_token;
}


void Parser::parse() 
{
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
    char* program_name = curr_token.val.string_value;
    std::cout << "Prog. name is " << program_name << '\n';

    require(TokenType::RS_IS);
}

void Parser::program_body()
{
    std::cout << "program body" << '\n';
    bool declarations = true;
    while (true)
    {
        if (token() == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            continue;
        }
        else if (token() == TokenType::RS_END)
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
    if (token() == TokenType::RS_GLOBAL)
    {
        global = true;
        advance();
    }

    if (token() == TokenType::RS_PROCEDURE)
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
    require(TokenType::IDENTIFIER);
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        parameter_list(); 
    require(TokenType::R_PAREN);
}

void Parser::proc_body()
{
    std::cout << "proc body" << '\n';
    bool declarations = true;
    while (true)
    {
        if (token() == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            continue;
        }
        else if (token() == TokenType::RS_END)
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
        if (token() == TokenType::COMMA)
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

    TokenType param_type = token(); // IN|OUT|INOUT
    std::cout << "Param type: " << TokenTypeStrings[param_type] << '\n';
    advance();
}


void Parser::var_declaration()
{
    std::cout << "var decl" << '\n';
    type_mark();
    char* id = require(TokenType::IDENTIFIER).val.string_value;
    std::cout << "Identifier: " << id << '\n';

    if (token() == TokenType::L_BRACKET)
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
    TokenType typemark = token();
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

void Parser::statement()
{
    std::cout << "stmnt" << '\n';

    if (token() == TokenType::IDENTIFIER)
        identifier_statement();
    else if (token() == TokenType::RS_IF)
        if_statement();
    else if (token() == TokenType::RS_FOR)
        loop_statement();
    else if (token() == TokenType::RS_RETURN)
        return_statement();
    else ; // TODO: ERR
}

// Groups assignment and proc call statements, as both
//  start with an identifier
void Parser::identifier_statement()
{
    std::cout << "identifier stmnt" << '\n';
    // Advance to next token; returning the current token
    //  and retrieving the identifier value
    const char* identifier = advance().val.string_value;
    if (token() == TokenType::ASSIGNMENT)
    {
        assignment_statement(identifier);
    }
    else if (token() == TokenType::L_PAREN)
    {
        proc_call(identifier);
    }
    else
    {
        //TODO err
    }
}

void Parser::assignment_statement(const char* identifier)
{
    std::cout << "assignment stmnt" << '\n';
    // already have identifier
    require(TokenType::ASSIGNMENT);
    expression();
}

void Parser::proc_call(const char* identifier)
{
    std::cout << "proc call" << '\n';
    // already have identifier
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        argument_list();
    require(TokenType::R_PAREN);
}

void Parser::if_statement()
{
    std::cout << "if" << '\n';
    require(TokenType::RS_IF);
    require(TokenType::L_PAREN);
    // TODO expression should return something
    expression();
    require(TokenType::R_PAREN);
    require(TokenType::RS_THEN);

    // TODO: Check condition
    //TODO handle else
    while (true)
    {
        // TODO: Make sure at least one statement exists
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
        // TODO control execution properly
        if (token() == TokenType::RS_ELSE) 
        {
            advance();
            continue;
        }
    }
    
    require(TokenType::RS_END);
    require(TokenType::RS_IF);
}

void Parser::loop_statement()
{
    std::cout << "loop" << '\n';
    require(TokenType::RS_FOR);
    require(TokenType::L_PAREN);
    require(TokenType::IDENTIFIER);
    assignment_statement(curr_token.val.string_value); 
    require(TokenType::SEMICOLON);
    expression();
    require(TokenType::R_PAREN);
    // TODO: Check conditions
    while (true)
    {
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
    }
    require(TokenType::RS_END); // Just to be sure 
    require(TokenType::RS_FOR);
}

void Parser::return_statement()
{
    std::cout << "return" << '\n';
    require(TokenType::RS_RETURN);
}

void Parser::argument_list()
{
    std::cout << "arg list" << '\n';
    while (true)
    {
        expression();
        if (token() == TokenType::COMMA) 
        {
            advance();
            continue;
        }
        else return;
    }
}

void Parser::destination()
{
    std::cout << "destination" << '\n';
    require(TokenType::IDENTIFIER);
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression();
        require(TokenType::R_BRACKET);
    }
}

void Parser::expression()
{
    std::cout << "expr" << '\n';

    arith_op(); 
    expression_pr();
}

void Parser::expression_pr()
{
    std::cout << "expr prime" << '\n';

    // TODO: Might need to separate & and | code for code generation stage
    if (token() == TokenType::AND || token() == TokenType::OR)
    {
        advance();
        arith_op();
        expression_pr();
    }
}

void Parser::arith_op()
{
    std::cout << "arith op" << '\n';
    if (token() == TokenType::RS_NOT)
    {
        advance();
    }

    relation();
    arith_op_pr();
}

void Parser::arith_op_pr()
{
    std::cout << "arith op pr" << '\n';
    if (token() == TokenType::PLUS || token() == TokenType::MINUS)
    {
        advance();
        relation(); 
        arith_op_pr();
    }
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
    if ((token() == TokenType::LT)
        | (token() == TokenType::GT)
        | (token() == TokenType::LT_EQ)
        | (token() == TokenType::GT_EQ)
        | (token() == TokenType::EQUALS)
        | (token() == TokenType::NOTEQUAL))
    {
        advance();
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

    if ((token() == TokenType::MULTIPLICATION)
        | (token() == TokenType::DIVISION))
    {
        advance();
        factor();
        term_pr();
    }
}

void Parser::factor()
{
    std::cout << "factor" << '\n';

    // Token is either (expression), name, number, string, char, true, false
    if (token() == TokenType::L_PAREN)
    {
        advance();
        expression();
        require(TokenType::R_PAREN);
    }
    else if (token() == TokenType::MINUS)
    {
        advance();
        // Name or number
        if (token() != TokenType::NUMBER)
            name();
    }
    else if (token() == TokenType::STRING 
            || token() == TokenType::CHAR 
            || token() == TokenType::RS_TRUE 
            || token() == TokenType::RS_FALSE 
            || token() == TokenType::NUMBER)
    {
        advance();
        // TODO: just return the value?
    }
    else 
    {
        name();
    }
}

void Parser::name()
{
    std::cout << "name" << '\n';
    require(TokenType::IDENTIFIER);
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression();
        require(TokenType::R_BRACKET);
    }
}


