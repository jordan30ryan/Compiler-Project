#pragma once
#include "token.h"
#include "errhandler.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"

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
    TokenType type; 

    // if type==IDENTIFIER, 
    // stores the llvm variable allocated space
    //llvm::AllocaInst* value;
    llvm::Value* value;
    
    // Maybe just keep this? No need for the other info then?
    llvm::Function* function;

    SymbolType sym_type = S_UNDEFINED;

    // If this is a procedure, this
    //  stores the proc local symbol table
    SymTable* local_symbols;

    // If this is a procedure, stores a list of params: type, id, in|out|inout
    std::vector<SymTableEntry*> parameters;

    // For a parameter
    // One of in | out | inout
    TokenType param_type;

    // For reserved words 
    SymTableEntry(TokenType t) : type(t) {}
    // For identifers where the type is known
    SymTableEntry(TokenType t, SymbolType st) : type(t), sym_type(st) {}
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
    SymTableEntry* resolve_symbol(std::string id, bool check=true); 

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
};

