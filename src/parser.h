#pragma once
#include "scanner.h"

class Parser
{
public:
    Parser(Scanner* scan);
    void parse();
private:
    Scanner* scanner;
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

