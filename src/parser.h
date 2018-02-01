#pragma once
#include "scanner.h"

class Parser
{
public:
    Parser(Scanner* scan);
    void parse();
private:
    Scanner* scanner;

    // Get a token from scanner and store in curr_token (also return that same token)
    Token getToken();
    // Get a token, and check the type. If wrong type, throw error
    Token getToken(TokenType type);

    Token curr_token;
    Token next_token;

    void program();
    void program_header();
    void program_body();

    void identifier();
    void declaration();

    void proc_declaration();
    void proc_header();
    void proc_body();
    void parameter_list();
    void parameter();

    void var_declaration();
    void type_mark();
    void lower_bound();
    void upper_bound();

    void statement();
    void assignment_statement();
    void if_statement();
    void loop_statement();
    void return_statement();
    void proc_call();

    void argument_list();
    void destination();

    void expression();
    void arith_op();
    void relation();
    void term();
    void factor();
    void name();
    void number();
    void string_literal();
    void char_literal();


};

