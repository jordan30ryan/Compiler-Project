#pragma once
#include "token.h" 
#include "errhandler.h"
#include "symboltable.h"
#include "scanner.h"

#include <iostream>
#include <sstream>

class Parser
{
public:
    Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan);
    ~Parser();
    void parse();
private:
    // The stream to output LLVM to. Can be a file or stdout (for debugging)
    std::ostream* llvm_out;

    // For use in LLVM assembly codegen. 
    // Get next unnamed register.
    std::string next_reg();
    int reg_no = 0; // Current numbered register.

    ErrHandler* err_handler;
    SymbolTableManager* symtable_manager;
    Scanner* scanner;

    // Get a token from scanner and store in curr_token if !curr_token_valid
    TokenType token();
    // Consume the token; subsequent calls to getToken will return a new token.
    Token advance();
    // Ensure current_token has type t, 
    //  if not, report err (if error=true) or warning 
    Token require(TokenType t, bool error=true);


    // Convert val to expected, if possible.
    void convertType(Value& val, SymbolType expected);

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

    SymTableEntry* var_declaration(bool is_global);
    void type_mark();
    Value lower_bound();
    Value upper_bound();

    bool statement();
    void identifier_statement();
    void assignment_statement(std::string);
    void proc_call(std::string);
    void argument_list(SymTableEntry* proc_entry);

    void if_statement();
    void loop_statement();
    void return_statement();

    Value expression(SymbolType hintType);
    // _pr functions are needed for eliminating left recursion
    Value expression_pr(Value lhs, SymbolType hintType); 
    Value arith_op(SymbolType hintType);
    Value arith_op_pr(Value lhs, SymbolType hintType); 
    Value relation(SymbolType hintType);
    Value relation_pr(Value lhs, SymbolType hintType); 
    Value term(SymbolType hintType);
    Value term_pr(Value lhs, SymbolType hintType);
    Value factor(SymbolType hintType);
    Value name(SymbolType hintType);
};

