#include "symboltable.h"

SymbolTableManager::SymbolTableManager(ErrHandler* handler) : err_handler(handler) {}

SymTableEntry* SymbolTableManager::resolve_symbol(std::string id)
{
    SymTableEntry* entry = NULL;
    if (curr_symbols->count(id) != 0)
    {
        // It's in the current scope's symbol table
        entry = (*curr_symbols)[id];
    }
    else if (global_symbols.count(id) != 0)
    {
        // It's in the global symbol table
        entry = global_symbols[id];
    }
    else 
    {
        std::ostringstream stream;
        stream << "Identifier " << id << " not defined.";
        err_handler->reportError(stream.str());  
    }
    return entry;
}

void SymbolTableManager::add_symbol(std::string id, TokenType type, bool is_global)
{
    if (is_global) global_symbols.insert({id, new SymTableEntry(type)});
    else curr_symbols->insert({id, new SymTableEntry(type)});
}

void SymbolTableManager::add_symbol(std::string id, SymbolType type, bool is_global)
{
    if (is_global) global_symbols.insert({id, new SymTableEntry(type)});
    else curr_symbols->insert({id, new SymTableEntry(type)});
}

void SymbolTableManager::set_proc_scope(std::string id)
{
    // TODO: Check if proc was already declared
    (*curr_symbols)[id]->sym_type = S_PROCEDURE;
    (*curr_symbols)[id]->local_symbols = new SymTable();

    scope_stack.push(curr_symbols);

    curr_symbols = (*curr_symbols)[id]->local_symbols; 
    // curr_symbols is now the proc's scope's symbol table; 
    //  add itself to support recursion
    curr_symbols->insert({id, new SymTableEntry(S_PROCEDURE)});
}


void SymbolTableManager::reset_scope()
{
    curr_symbols = scope_stack.top();
    scope_stack.pop();
}

