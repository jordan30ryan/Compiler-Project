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
    Parser(ErrHandler* handler, 
        SymbolTableManager* manager, 
        Scanner* scan, 
        std::string filename);

    ~Parser();
    std::unique_ptr<llvm::Module> parse();
private:
    // For use in LLVM codegen. 
    // Declare builtin functions in the LLVM IR
    void decl_single_builtin(std::string name, llvm::Type* paramtype);
    void decl_builtins();
    std::string next_label();

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
    llvm::Value* convert_type(llvm::Value* val, llvm::Type* required_type);

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

    llvm::Value* expression(llvm::Type* hintType);
    // _pr functions are needed for eliminating left recursion
    llvm::Value* expression_pr(llvm::Value* lhs, llvm::Type* hintType); 
    llvm::Value* arith_op(llvm::Type* hintType);
    llvm::Value* arith_op_pr(llvm::Value* lhs, llvm::Type* hintType); 
    llvm::Value* relation(llvm::Type* hintType);
    llvm::Value* relation_pr(llvm::Value* lhs, llvm::Type* hintType); 
    llvm::Value* term(llvm::Type* hintType);
    llvm::Value* term_pr(llvm::Value* lhs, llvm::Type* hintType);
    llvm::Value* factor(llvm::Type* hintType);
    llvm::Value* name(llvm::Type* hintType);
};

