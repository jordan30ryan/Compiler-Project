#include "symboltable.h"

SymbolTableManager::SymbolTableManager(ErrHandler* handler) : err_handler(handler) {}

SymTableEntry* SymbolTableManager::resolve_symbol(std::string id, bool check, TokenType paramIntent)
{
    // exists but not well-defined, and is expected to be (check ==true)    
    bool check_err = false; 

    SymTableEntry* entry = NULL;

    if (curr_symbols->count(id) != 0)
    {
        // It's in the current scope's symbol table
        entry = (*curr_symbols)[id];
        if (check && entry->sym_type == S_UNDEFINED)
        {
            // We're checking for it to exist but it's type is undefined
            check_err = true;
        }

        if (entry->param_type != TokenType::UNKNOWN)
        {
            // Entry is a parameter; check its type and the intent of 
            //  the function using this variable

            if (entry->param_type == TokenType::RS_IN && paramIntent == RS_OUT)
                err_handler->reportError("Cannot write to a read-only parameter");
            else if (entry->param_type == TokenType::RS_OUT && paramIntent == RS_IN)
                err_handler->reportError("Cannot read from a write-only variable");
            else if (entry->param_type == TokenType::RS_INOUT || paramIntent == TokenType::UNKNOWN)
                ;// Not an error; it's r/w so all operations are permitted.
        }
    }
    else if (global_symbols.count(id) != 0)
    {
        // It's in the global symbol table
        entry = global_symbols[id];
        if (check && entry->sym_type == S_UNDEFINED)
        {
            // We're checking for it to exist but it's type is undefined
            check_err = true;
        }
    }
    else if (check) check_err = true;

    if (check_err)
    {
        std::ostringstream stream;
        stream << "Identifier " << id << " not defined.";
        err_handler->reportError(stream.str());  
    }

    return entry;
}

void SymbolTableManager::init_tables()
{
    // Init reserved words table
    add_symbol(true, "IN", TokenType::RS_IN);
    add_symbol(true, "OUT", TokenType::RS_OUT);
    add_symbol(true, "INOUT", TokenType::RS_INOUT);
    add_symbol(true, "PROGRAM", TokenType::RS_PROGRAM);
    add_symbol(true, "IS", TokenType::RS_IS);
    add_symbol(true, "BEGIN", TokenType::RS_BEGIN);
    add_symbol(true, "END", TokenType::RS_END);
    add_symbol(true, "GLOBAL", TokenType::RS_GLOBAL);
    add_symbol(true, "PROCEDURE", TokenType::RS_PROCEDURE);
    add_symbol(true, "STRING", TokenType::RS_STRING);
    add_symbol(true, "CHAR", TokenType::RS_CHAR);
    add_symbol(true, "INTEGER", TokenType::RS_INTEGER);
    add_symbol(true, "FLOAT", TokenType::RS_FLOAT);
    add_symbol(true, "BOOL", TokenType::RS_BOOL);
    add_symbol(true, "IF", TokenType::RS_IF);
    add_symbol(true, "THEN", TokenType::RS_THEN);
    add_symbol(true, "ELSE", TokenType::RS_ELSE);
    add_symbol(true, "FOR", TokenType::RS_FOR);
    add_symbol(true, "RETURN", TokenType::RS_RETURN);
    add_symbol(true, "TRUE", TokenType::RS_TRUE);
    add_symbol(true, "FALSE", TokenType::RS_FALSE);
    add_symbol(true, "NOT", TokenType::RS_NOT);

    add_builtin_proc(true, "GETBOOL", IDENTIFIER, S_PROCEDURE, S_BOOL, RS_OUT);
    add_builtin_proc(true, "GETINTEGER", IDENTIFIER, S_PROCEDURE, S_INTEGER, RS_OUT);
    add_builtin_proc(true, "GETFLOAT", IDENTIFIER, S_PROCEDURE, S_FLOAT, RS_OUT);
    add_builtin_proc(true, "GETSTRING", IDENTIFIER, S_PROCEDURE, S_STRING, RS_OUT);
    add_builtin_proc(true, "GETCHAR", IDENTIFIER, S_PROCEDURE, S_CHAR, RS_OUT);

    add_builtin_proc(true, "PUTBOOL", IDENTIFIER, S_PROCEDURE, S_BOOL, RS_IN);
    add_builtin_proc(true, "PUTINTEGER", IDENTIFIER, S_PROCEDURE, S_INTEGER, RS_IN);
    add_builtin_proc(true, "PUTFLOAT", IDENTIFIER, S_PROCEDURE, S_FLOAT, RS_IN);
    add_builtin_proc(true, "PUTSTRING", IDENTIFIER, S_PROCEDURE, S_STRING, RS_IN);
    add_builtin_proc(true, "PUTCHAR", IDENTIFIER, S_PROCEDURE, S_CHAR, RS_IN);
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
        if (is_global) global_symbols.insert({id, new SymTableEntry(type, stype, id)});
        else curr_symbols->insert({id, new SymTableEntry(type, stype, id)});
    }
}

void SymbolTableManager::add_builtin_proc(bool is_global, std::string id,
                                            TokenType type, SymbolType stype, 
                                            SymbolType param_sym_type, 
                                            TokenType param_type)
{
    // Add proc first
    add_symbol(is_global, id, type, stype);
    // Get proc
    SymTableEntry* proc_entry = resolve_symbol(id, true);

    // Setup proc's parameter
    SymTableEntry* param_entry = new SymTableEntry(IDENTIFIER, param_sym_type, id);
    param_entry->param_type = param_type; // IN|OUT|INOUT

    // Add parameter to proc's parameters
    proc_entry->parameters.push_back(param_entry);
    proc_entry->local_symbols = new SymTable();
    // Add parameter to proc's scope, named val.
    (*(proc_entry->local_symbols))["val"] = param_entry;
}

void SymbolTableManager::promote_to_global(std::string id, SymTableEntry* entry)
{
    if (scope_stack.size() != 0)
    {
        err_handler->reportWarning(
            "Global symbols can only be declared in the outermost scope. ");
    }
    else
    {
        if (entry == NULL)
        {
            err_handler->reportError(
                "Variable cannot be promoted to global scope before definition.");
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
    SymTableEntry* proc_entry = (*curr_symbols)[id];

    // TODO: Check if proc was already declared
    proc_entry->sym_type = S_PROCEDURE;
    proc_entry->local_symbols = new SymTable();

    scope_stack.push({curr_symbols, curr_proc});
    curr_symbols = proc_entry->local_symbols; 
    curr_proc = proc_entry;
    // curr_symbols is now this procedure's scope's symbol table.
    //  everything defined now in curr_symbols 
    //  will be defined in this procedure's scope.
    // curr_proc is now the SymTableEntry of this proc.

    // Add this proc to its own symbol table to support recursion
    curr_symbols->insert({id, proc_entry});
}

void SymbolTableManager::add_param_to_proc(SymTableEntry* param_entry)
{
    if (curr_proc != NULL)
        curr_proc->parameters.push_back(param_entry);
    else; // TODO: This is probably an error...
}

std::vector<SymTableEntry*> SymbolTableManager::get_current_proc_params()
{
    if (curr_proc != NULL) return curr_proc->parameters;
    else 
        // TODO: This is probably an error...
        return std::vector<SymTableEntry*>();
}

llvm::IRBuilderBase::InsertPoint SymbolTableManager::get_insert_point()
{
    if (curr_proc != NULL)
    {
        return curr_proc->ip;
    }
    else return global_ip; 
}

void SymbolTableManager::save_insert_point(llvm::IRBuilderBase::InsertPoint ip)
{
    if (curr_proc != NULL)
        curr_proc->ip = ip;
    else
        global_ip = ip;
}

void SymbolTableManager::set_curr_proc_function(llvm::Function* F)
{
    if (curr_proc != NULL)
    {
        curr_proc->function = F;
    }
}

void SymbolTableManager::reset_scope()
{
    std::pair<SymTable*, SymTableEntry*> context = scope_stack.top();
    curr_symbols = context.first;
    curr_proc = context.second;
    scope_stack.pop();
}

