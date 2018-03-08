#pragma once
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>

#include "token.h" 
#include "errhandler.h"
#include "symboltable.h"
#include "scanner.h"

using namespace llvm;
using namespace llvm::sys;


class Parser
{
public:
    Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan);
    void parse();
private:
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

