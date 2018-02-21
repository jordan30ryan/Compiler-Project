#pragma once
#include "scanner.h"
#include "token.h" 

#include <iostream>
#include <sstream>
#include <stack>

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

    void proc_declaration(bool is_global);
    void proc_header();
    void proc_body();
    void parameter_list();
    void parameter();

    void var_declaration(bool is_global);
    void type_mark();
    void lower_bound();
    void upper_bound();

    bool statement();
    void identifier_statement();
    void proc_call(std::string);
    void assignment_statement(std::string);
    void if_statement();
    void loop_statement();
    void return_statement();

    void argument_list();
    void destination();

    Value expression();
    Value expression_pr(Value lhs); // needed for eliminating left recursion
    Value arith_op();
    Value arith_op_pr(Value lhs); // needed for eliminating left recursion
    Value relation();
    Value relation_pr(Value lhs); // needed for eliminating left recursion
    Value term();
    Value term_pr(Value lhs);
    Value factor();
    Value name();
};

