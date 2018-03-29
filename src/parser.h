#pragma once
#include "token.h" 
#include "errhandler.h"
#include "symboltable.h"
#include "scanner.h"
#include "llvm_helper.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstring>

class Parser
{
public:
    Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan, std::string filename);
    ~Parser();
    //void parse();
    std::unique_ptr<llvm::Module> parse();
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
    //std::string get_val(MyValue val);
    // Get next unnamed register.
    //std::string next_reg();
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
    void convert_type(MyValue& val, std::string& val_reg_str, SymbolType required_type);

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
    llvm::Value* lower_bound();
    llvm::Value* upper_bound();

    bool statement();
    void identifier_statement();
    void assignment_statement(std::string);
    void proc_call(std::string);
    std::vector<llvm::Value*> argument_list(SymTableEntry* proc_entry);

    void if_statement();
    void loop_statement();
    void return_statement();

    llvm::Value* expression(SymbolType hintType);
    // _pr functions are needed for eliminating left recursion
    llvm::Value* expression_pr(llvm::Value* lhs, SymbolType hintType); 
    llvm::Value* arith_op(SymbolType hintType);
    llvm::Value* arith_op_pr(llvm::Value* lhs, SymbolType hintType); 
    llvm::Value* relation(SymbolType hintType);
    llvm::Value* relation_pr(llvm::Value* lhs, SymbolType hintType); 
    llvm::Value* term(SymbolType hintType);
    llvm::Value* term_pr(llvm::Value* lhs, SymbolType hintType);
    llvm::Value* factor(SymbolType hintType);
    llvm::Value* name(SymbolType hintType);
};

