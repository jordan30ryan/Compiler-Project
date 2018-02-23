#include "symboltable.h"

SymbolTableManager::SymbolTableManager(ErrHandler* handler) : err_handler(handler) {}

SymTableEntry* SymbolTableManager::resolve_symbol(std::string id, bool check)
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
    else if (check)
    {
        std::ostringstream stream;
        stream << "Identifier " << id << " not defined.";
        err_handler->reportError(stream.str());  
    }
    return entry;
}

void SymbolTableManager::add_symbol(bool is_global, std::string id,
                                    TokenType type, SymbolType stype)
{
    if (is_global && scope_stack.size() != 0) 
    {
        err_handler->reportWarning("Global symbols can only be declared in the outermost scope. ");
        is_global = false; // Treat this as a local variable decl.
    }

    if ((*curr_symbols).count(id) != 0)
    {
        std::ostringstream stream;
        stream << "Identifier " << id << " already defined in local scope";
        err_handler->reportError(stream.str());
    }
    else if (global_symbols.count(id) != 0)
    {
        std::ostringstream stream;
        stream << "Identifier " << id << " already defined in global scope";
        err_handler->reportError(stream.str());
    }
    // Only insert when not defined yet.
    else     
    {
        if (is_global) global_symbols.insert({id, new SymTableEntry(type, stype)});
        else curr_symbols->insert({id, new SymTableEntry(type, stype)});
    }
}

void SymbolTableManager::promote_to_global(std::string id)
{
    if (scope_stack.size() != 0)
    {
        err_handler->reportWarning("Global symbols can only be declared in the outermost scope. ");
    }
    else
    {
        SymTableEntry* entry = resolve_symbol(id);
        if (entry == NULL)
        {
            err_handler->reportError("Variable cannot be promoted to global scope before definition.");
        }
        else 
        {
            // Add entry into global_symbols
            // TODO: ok to leave entry in local scope also?
            global_symbols[id] = entry;
        }
    }
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
    curr_symbols->insert({id, new SymTableEntry(IDENTIFIER, S_PROCEDURE)});
}


void SymbolTableManager::reset_scope()
{
    curr_symbols = scope_stack.top();
    scope_stack.pop();
}

