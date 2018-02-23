#pragma once
#include "token.h"
#include "errhandler.h"

#include <sstream>
#include <unordered_map>
#include <stack>

struct SymTableEntry;
typedef std::unordered_map<std::string, SymTableEntry*> SymTable;

// One entry in a SymTable. Can be an identifier (variable/procedure) 
//  or a reserved word mapped to its TokenType.
struct SymTableEntry
{
    TokenType type; 

    // if type==IDENTIFIER, 
    //  stores identifier type (i.e. Variable type or Procedure)
    SymbolType sym_type = S_UNDEFINED;

    //bool is_global;
    // If type==IDENTIFER && sym_type==S_PROCEDURE, 
    //  stores the proc local symbol table
    SymTable* local_symbols;

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
    void add_symbol(bool is_global, std::string id, TokenType type=UNKNOWN, SymbolType stype=S_UNDEFINED);
    // Promote a locally defined symbol to the global scope
    void promote_to_global(std::string id);
    // Sets the current_scope to the scope of the named procedure
    //  (for procedure definitions)
    void set_proc_scope(std::string id);
    // Reset curr_symbols to one scope up (pop from the stack)
    void reset_scope();

private:
    // Stack of symbol tables, becuase each proc can have multiple proc definitions
    std::stack<SymTable*> scope_stack;

    // The global scope symbol table
    SymTable global_symbols;
    // The local scope symbol table (varies depending on context)
    // The parser will be modifying this so it is guranteed to point to the 
    //  correct scope based on parser context
    SymTable* curr_symbols = new SymTable();

    ErrHandler* err_handler;
};
