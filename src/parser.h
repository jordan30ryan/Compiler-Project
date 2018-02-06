#pragma once
#include "scanner.h"

class Parser
{
public:
    Parser(Scanner* scan, ErrHandler* handler);
    void parse();
private:
    Scanner* scanner;
    ErrHandler* err_handler;

    // Get a token from scanner and store in curr_token if !curr_token_valid
    TokenType token();
    // Consume the token; subsequent calls to getToken will return a new token.
    Token advance();
    // Ensure current_token has type t; report err if not. 
    Token require(TokenType t);

    Token curr_token;
    bool curr_token_valid;

    void program();
    void program_header();
    void program_body();

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
    void identifier_statement();
    void proc_call(const char*);
    void assignment_statement(const char*);
    void if_statement();
    void loop_statement();
    void return_statement();

    void argument_list();
    void destination();

    void expression();
    void expression_pr(); // Needed for eliminating left recursion
    void arith_op();
    void arith_op_pr(); // Needed for eliminating left recursion
    void relation();
    void relation_pr(); // Needed for eliminating left recursion
    void term();
    void term_pr();
    void factor();
    void name();
    //void number();
    //void string_literal();
    //void char_literal();


};

