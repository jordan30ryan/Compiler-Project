#pragma once
#include "token.h"
#include "errhandler.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

#include <sstream>
#include <unordered_map>
#include <stack>
#include <vector>

struct SymTableEntry;
typedef std::unordered_map<std::string, SymTableEntry*> SymTable;

// One entry in a SymTable. Can be an identifier (variable/procedure) 
//  or a reserved word mapped to its TokenType.
struct SymTableEntry
{
    // For reserved words 
    SymTableEntry(TokenType t) : type(t) {}
    // For identifers where the type is known
    SymTableEntry(TokenType t, SymbolType st, std::string i) 
        : type(t), sym_type(st), id(i) {}

    TokenType type; 

    SymbolType sym_type = S_UNDEFINED;

    // Because these might be used in contexts other than the map
    std::string id;

    // If this is a procedure, this
    //  stores the proc local symbol table
    SymTable* local_symbols;

    // If this is a procedure, stores a list of params: type, id, in|out|inout
    std::vector<SymTableEntry*> parameters;

    // For a parameter
    // One of in | out | inout
    TokenType param_type = TokenType::UNKNOWN;


    // if type==IDENTIFIER, 
    // stores the llvm variable allocated space
    //llvm::AllocaInst* value;
    llvm::Value* value;
    
    // Maybe just keep this? No need for the other info then?
    llvm::Function* function;

    // If this is a function, this is the insert point that the builder
    //  should reset to when it needs to append to this function
    llvm::IRBuilderBase::InsertPoint ip;
};

class SymbolTableManager
{
public:
    SymbolTableManager(ErrHandler* err_handler);

    // Get a given identifier's associated symtable entry
    //  - can be global or current scope
    // If check is true - function expects the variable to exist and 
    //  reports an error if it does not.
    // WARNING RETURNS NULL AND REPORTS TO ERR_HANDLER IF NOT FOUND.
    // paramIntent - the intention for this symbol; used to check parameters
    // Parameters can be IN (read only) or OUT (write only) or INOUT (read/write)
    // The intent for the parameter can be IN (it wants to read) or OUT (it wants to write)
    // If we expect to be able to read but the type is OUT, it's an error
    // Same thing if we expect to write but the type is IN
    SymTableEntry* resolve_symbol(std::string id, bool check, TokenType paramIntent=TokenType::UNKNOWN); 

    // Setup the global table with reserved words and builtin procedures
    void init_tables();

    // Add a symbol to the current symbol table
    void add_symbol(bool is_global, std::string id, 
                    TokenType type=UNKNOWN, SymbolType stype=S_UNDEFINED);

    // Add a builtin proc to the global table with a single parameter
    // Helper function for init_tables().
    void add_builtin_proc(bool is_global, std::string id, TokenType type, 
                            SymbolType stype, SymbolType param_sym_type, 
                            TokenType param_type);

    // Promote a locally defined symbol to the global scope
    void promote_to_global(std::string id, SymTableEntry* entry);

    // Sets the current_scope to the scope of the named procedure
    //  (for procedure definitions)
    void set_proc_scope(std::string id);

    // Add a parameter to the current proc. If the current scope is a proc,
    //  report an error.
    //void add_param_to_proc(SymbolType typemark, std::string id, TokenType param_type);
    void add_param_to_proc(SymTableEntry* param_entry);

    // Get params of the current procedure
    std::vector<SymTableEntry*> get_current_proc_params();

    // Sets/Gets the insert point for the IR Builder to use for this function
    void save_insert_point(llvm::IRBuilderBase::InsertPoint ip);
    llvm::IRBuilderBase::InsertPoint get_insert_point();

    // Set the current procedure's llvm function
    void set_curr_proc_function(llvm::Function* F);

    // Reset curr_symbols to one scope up (pop from the stack)
    void reset_scope();

private:
    ErrHandler* err_handler;

    // The global scope symbol table
    SymTable global_symbols;
    // The local scope symbol table (varies depending on context)
    // The parser will be modifying this so it is guranteed to point to the 
    //  correct scope based on parser context
    SymTable* curr_symbols = new SymTable();

    // The current procedure, if the current scope is a procedure's scope.
    // If current scope is the outer scope, this is null.
    SymTableEntry* curr_proc = NULL;

    // Stack of symbol tables, becuase each proc can have multiple proc definitions
    // The second element of the pair is the entry for the proc's scope itself.
    // (essentially, this tracks curr_symbols and curr_proc for inner scopes)
    std::stack<std::pair<SymTable*, SymTableEntry*>> scope_stack;

    // The outermost insert point. Setting insert point to this in the Builder
    //  will generate code in main of the output IR
    llvm::IRBuilderBase::InsertPoint global_ip;
};

