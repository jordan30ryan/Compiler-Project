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
    Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan, std::string filename);
    ~Parser();
    void parse();
private:
    // The stream to output LLVM to. Can be a file or stdout (for debugging)
    std::ofstream llvm_out;

    // The codegen output. Can be llvm_out or an ostringstream to be appended
    //  to llvm_out at the end (for procedures)
    std::ostream* codegen_out;
    std::vector<std::ostringstream*> procedure_defs;
    std::stack<std::ostream*> stream_stack;

    // For use in LLVM assembly codegen. 
    // Declare builtin functions in the llvm output file
    void decl_builtins();
    // Get value of val; can be either a literal or a register
    // If it's a variable, it loads from the variable's pointer and returns a temp register.
    // If it's a literal, it just returns the literal in string form for use in llvm generation
    std::string get_val(Value val);
    // Get next unnamed register.
    std::string next_reg();
    int reg_no = 0; // Current numbered register.
    // Like next_reg but for labels
    std::string next_label();
    int label_no = 0;

    ErrHandler* err_handler;
    SymbolTableManager* symtable_manager;
    Scanner* scanner;

    Token curr_token;
    bool curr_token_valid;

    // Get a token from scanner and store in curr_token if !curr_token_valid
    TokenType token();
    // Consume the token; subsequent calls to getToken will return a new token.
    Token advance();
    // Ensure current_token has type t, 
    //  if not, report err (if error=true) or warning 
    Token require(TokenType t, bool error=true);

    // For type conversion
    // Val - the value to convert.
    // reg_str - the register of val
    // required_type - the type to convert to.
    void convert_type(Value& val, std::string& val_reg_str, SymbolType required_type);

    void program();
    void program_header();
    void program_body();

    void declaration();

    void proc_declaration(bool is_global);
    void proc_header();
    void proc_body();
    void parameter_list();
    void parameter();

    SymTableEntry* var_declaration(bool is_global, bool need_alloc=true);
    void type_mark();
    Value lower_bound();
    Value upper_bound();

    bool statement();
    void identifier_statement();
    void assignment_statement(std::string);
    void proc_call(std::string);
    std::vector<Value> argument_list(SymTableEntry* proc_entry);

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

