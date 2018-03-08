#include "parser.h"
#define DEBUG false

// To assist in error printing 
const char* TokenTypeStrings[] = 
{
".", ";", "(", ")", ",", "[", "]", ":", "&", "|", "+", "-", "<", ">", "<=", ">=", ":=", "==", "!=", "*", "/", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "IDENTIFIER", "UNKNOWN",
"RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT"
};

const char* SymbolTypeStrings[] = 
{
    "S_UNDEFINED", "S_STRING", "S_CHAR", "S_INTEGER", "S_FLOAT", "S_BOOL", "S_PROCEDURE"
};


Parser::Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan)
    : err_handler(handler), symtable_manager(manager), scanner(scan) 
{ 
    // Initialize curr_token so old values aren't used 
    curr_token.type = UNKNOWN;
    curr_token.val.sym_type = S_UNDEFINED;
}

TokenType Parser::token()
{
    if (curr_token.type == TokenType::FILE_END)
    {
        // Possibly desynchronized - this would go on infinitely, terminate here
        throw std::runtime_error("Repeated attempt to get FILE_END token. Terminating due to desynchronization.");
    }
    if (!curr_token_valid)
    {
        curr_token_valid = true;
        curr_token = scanner->getToken();
        if (DEBUG) std::cout << "\tGot: " << TokenTypeStrings[curr_token.type] << '\n';
        return curr_token.type;
    }
    else return curr_token.type;
}

Token Parser::advance()
{
    // Mark as consumed
    curr_token_valid = false;
    // Return the current token to be used before getting next token 
    return curr_token;
}

// Require that the current token is of type t
Token Parser::require(TokenType expected_type, bool error)
{
    TokenType type = token();
    advance();
    if (expected_type != type) 
    {
        // Report err
        std::ostringstream stream;
        stream << "Bad Token: " << TokenTypeStrings[type] 
            << "\tExpected: " << TokenTypeStrings[expected_type];
        if (error) err_handler->reportError(stream.str(), curr_token.line);
        else err_handler->reportWarning(stream.str(), curr_token.line);
    }
    return curr_token;
}

// TODO: This doesn't really work ???
/*
// Convert val to expected, if possible.
void Parser::convertType(Value& val, SymbolType expected)
{
    if (val.sym_type == expected) return;

    switch (expected)
    {
    case S_INTEGER:
        if (val.sym_type == S_FLOAT) 
        {
            val.int_value = val.float_value;
            val.sym_type = S_INTEGER;
        }
        else if (lhs.sym_type == S_BOOL)
        {
            // bool value is already stored in int_value
            val.sym_type = S_INTEGER;
        }
        else 
        {
            // TODO: error. can't convert anything else to int
        }
        break;
    case S_BOOL:
        if (lhs.sym_type == S_INTEGER) 
        {
            // bool value is already stored in int_value
            lhs.sym_type = S_BOOL;
        }
        else 
        {
            // TODO: error. can't convert anything else to bool
        }
        break;
    case S_FLOAT:
        // TODO
        if (lhs.sym_type == S_INTEGER)
        break;
    case S_CHAR:
        break;
    case S_STRING:
        break;
    default:
        break;
    }
}
*/

void Parser::parse() 
{
    bool synchronized = false;
    try 
    {
        program();
        synchronized = true;
        // This may cause the exception (repeatedly getting FILE_END)
        // BUT, this indicates synchronization, so an error isn't reported.
        require(TokenType::FILE_END);
    }
    catch (std::runtime_error& err)
    {
        if (!synchronized) err_handler->reportError(err.what());
    }
}

void Parser::program()
{
    if (DEBUG) std::cout << "program" << '\n';
    program_header(); 
    program_body(); 
    require(TokenType::PERIOD, false);
}

void Parser::program_header()
{
    if (DEBUG) std::cout << "program header" << '\n';
    require(TokenType::RS_PROGRAM);

    require(TokenType::IDENTIFIER);
    std::string program_name = curr_token.val.string_value;
    std::cout << "Prog. name is " << program_name << '\n';

    require(TokenType::RS_IS);
}

void Parser::program_body()
{
    if (DEBUG) std::cout << "program body" << '\n';
    bool declarations = true;
    while (true)
    {
        if (token() == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            continue;
        }
        else if (token() == TokenType::RS_END)
        {
            advance();
            require(TokenType::RS_PROGRAM);
            return;
        }
        
        if (declarations) declaration();
        else statement();

        require(TokenType::SEMICOLON);
    }
}

void Parser::declaration()
{
    if (DEBUG) std::cout << "declaration" << '\n';
    bool is_global = false;
    if (token() == TokenType::RS_GLOBAL)
    {
        is_global = true;
        advance();
    }

    if (token() == TokenType::RS_PROCEDURE)
    {
        proc_declaration(is_global);
    }
    else var_declaration(is_global); // typemark - continue into var decl
}

void Parser::proc_declaration(bool is_global)
{
    if (DEBUG) std::cout << "proc decl" << '\n';
    proc_header();
    proc_body();

    // Reset to scope above this proc decl
    symtable_manager->reset_scope();
}

void Parser::proc_header()
{
    if (DEBUG) std::cout << "proc header" << '\n';
    require(TokenType::RS_PROCEDURE);

    // Setup symbol table so the procedure's sym table is now being used
    std::string proc_id = require(TokenType::IDENTIFIER).val.string_value;
    // Sets the current scope to this procedure's scope
    symtable_manager->set_proc_scope(proc_id);

    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        parameter_list(); 
    require(TokenType::R_PAREN);
}

void Parser::proc_body()
{
    if (DEBUG) std::cout << "proc body" << '\n';
    bool declarations = true;
    while (true)
    {
        if (token() == TokenType::RS_BEGIN)
        {
            advance();
            declarations = false;
            continue;
        }
        else if (token() == TokenType::RS_END)
        {
            advance();
            require(TokenType::RS_PROCEDURE);
            return;
        }
        
        if (declarations) declaration();
        else statement();

        require(TokenType::SEMICOLON);
    }
}

void Parser::parameter_list()
{
    if (DEBUG) std::cout << "param list" << '\n';
    while (true)
    {
        parameter(); 
        if (token() == TokenType::COMMA)
        {
            advance();
            continue;
        }
        else return;
    }
}

void Parser::parameter()
{
    if (DEBUG) std::cout << "param" << '\n';

    // curr_token is the typemark

    SymTableEntry* entry = var_declaration(false);

    if (token() != TokenType::RS_IN
        && token() != TokenType::RS_OUT
        && token() != TokenType::RS_INOUT)
    {
        err_handler->reportError("Parameter passing type must be one of: IN or OUT or INOUT", curr_token.line);
    }
    entry->param_type = token(); // IN|OUT|INOUT
    advance();

    symtable_manager->add_param_to_proc(entry);
}


SymTableEntry* Parser::var_declaration(bool is_global)
{
    if (DEBUG) std::cout << "var decl" << '\n';
    // This is the only place in grammar type mark occurs 
    //  so it doesn't need its own function
    TokenType typemark = token();
    advance();

    std::string id = require(TokenType::IDENTIFIER).val.string_value;
    SymTableEntry* entry = symtable_manager->resolve_symbol(id);
    if (entry != NULL && entry->sym_type != S_UNDEFINED)
    {
        std::ostringstream stream;
        stream << "Variable " << id << " may have already been defined the local or global scope.";
        err_handler->reportError(stream.str(), curr_token.line);
    }

    // TODO: Better way to do this? lol
    switch (typemark)
    {
    case RS_STRING:
        entry->sym_type = S_STRING;
        break;
    case RS_CHAR:
        entry->sym_type = S_CHAR;
        break;
    case RS_INTEGER:
        entry->sym_type = S_INTEGER;
        break;
    case RS_FLOAT:
        entry->sym_type = S_FLOAT;
        break;
    case RS_BOOL:
        entry->sym_type = S_BOOL;
        break;
    default:
        std::ostringstream stream;
        stream << "Unknown typemark: " << TokenTypeStrings[typemark];
        err_handler->reportError(stream.str(), curr_token.line);
        break;
    }

    // Only insert into global symbols if prefixed with RS_GLOBAL (is_global == true)
    if (is_global) symtable_manager->promote_to_global(id, entry);

    // Indexing
    if (token() == TokenType::L_BRACKET)
    {
        advance();

        Value lower = lower_bound();
        require(TokenType::COLON);
        Value upper = upper_bound();

        require(TokenType::R_BRACKET);
        // TODO: setup the variable as an array
    }

    return entry;
}

Value Parser::lower_bound()
{
    if (DEBUG) std::cout << "lower_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return require(TokenType::INTEGER).val;
}

Value Parser::upper_bound()
{
    if (DEBUG) std::cout << "upper_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return require(TokenType::INTEGER).val;
}

bool Parser::statement()
{
    if (DEBUG) std::cout << "stmnt" << '\n';

    if (token() == TokenType::IDENTIFIER)
        identifier_statement();
    else if (token() == TokenType::RS_IF)
        if_statement();
    else if (token() == TokenType::RS_FOR)
        loop_statement();
    else if (token() == TokenType::RS_RETURN)
        return_statement();
    else return false;

    return true;
}

// Groups assignment and proc call statements, as both
//  start with an identifier
void Parser::identifier_statement()
{
    if (DEBUG) std::cout << "identifier stmnt" << '\n';
    // Advance to next token; returning the current token
    //  and retrieving the identifier value
    std::string identifier = advance().val.string_value;
    
    if (token() == TokenType::L_PAREN)
    {
        proc_call(identifier);
    }
    else
    {
        assignment_statement(identifier);
    }
}

void Parser::assignment_statement(std::string identifier)
{
    if (DEBUG) std::cout << "assignment stmnt" << '\n';
//    std::cout << "iden:" << identifier 
//                << "\tType: " << (*curr_symbols)[identifier]->sym_type << '\n';

    // already have identifier; need to check for indexing first
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression(SymbolType::S_INTEGER);
        require(TokenType::R_BRACKET);
    }
    SymTableEntry* entry = symtable_manager->resolve_symbol(identifier); 
    // TODO: something with indexing
    require(TokenType::ASSIGNMENT);
    Value rhs = expression(entry->sym_type);
    if (entry->sym_type != rhs.sym_type)
    {
        //TODO
    }
}

void Parser::proc_call(std::string identifier)
{
    if (DEBUG) std::cout << "proc call" << '\n';
    // already have identifier

    // Check symtable for the proc
    SymTableEntry* proc_entry = symtable_manager->resolve_symbol(identifier); 
    if (proc_entry != NULL && proc_entry->sym_type == S_PROCEDURE)
    {
        // TODO: call the proc defined by proc_entry in the symtable
    }
    else
    {
        std::ostringstream stream;
        stream << "Procedure " << identifier << " not defined\n";
        err_handler->reportError(stream.str(), curr_token.line);
    }

    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        argument_list(proc_entry);
    require(TokenType::R_PAREN);
}

void Parser::argument_list(SymTableEntry* proc_entry)
{
    if (DEBUG) std::cout << "arg list" << '\n';
    for (auto param : proc_entry->parameters)
    {
        Value val = expression(param->sym_type);
        if (val.sym_type != param->sym_type)
        {
            // TODO: Some unmatched types can be converted:
            //  Integer<->Float
            //  Integer->Bool
            //  Bool->Integer ????
            //  Integer<->Char ????
            std::ostringstream stream;
            stream 
                << "Procedure call paramater type doesn't match expected type."
                << "\n\tGot:\t\t" << SymbolTypeStrings[val.sym_type] 
                << "\n\tExpected:\t" << SymbolTypeStrings[param->sym_type];
            err_handler->reportError(stream.str());
        }

        if (token() == TokenType::COMMA) 
        {
            advance();
            continue;
        }
        else return;
    }
}

void Parser::if_statement()
{
    if (DEBUG) std::cout << "if" << '\n';
    require(TokenType::RS_IF);

    require(TokenType::L_PAREN);
    Value condition = expression(SymbolType::S_BOOL);
    require(TokenType::R_PAREN);

    require(TokenType::RS_THEN);

    //TODO handle else (code generation stage)
    bool first_stmnt = true;
    while (true)
    {
        // Make sure there is at least one valid statement
        bool valid = statement();
        if (first_stmnt && !valid)
        {
            err_handler->reportError("No statement in IF body");
        }
        first_stmnt = false;
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
        if (token() == TokenType::RS_ELSE) 
        {
            advance();
            continue;
        }
    }
    
    require(TokenType::RS_END);
    require(TokenType::RS_IF);
}

void Parser::loop_statement()
{
    if (DEBUG) std::cout << "loop" << '\n';
    require(TokenType::RS_FOR);

    require(TokenType::L_PAREN);
    require(TokenType::IDENTIFIER);
    assignment_statement(curr_token.val.string_value); 
    require(TokenType::SEMICOLON);
    Value condition = expression(SymbolType::S_BOOL);
    require(TokenType::R_PAREN);

    while (true)
    {
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
    }
    require(TokenType::RS_END); // Just to be sure, also to advance the token
    require(TokenType::RS_FOR);
}

void Parser::return_statement()
{
    if (DEBUG) std::cout << "return" << '\n';
    require(TokenType::RS_RETURN);
}

// hintType - the expected type (e.g. if this is an assignment)
//  used as a hint to expression on what type to convert to 
//  (if type conversion is needed)
Value Parser::expression(SymbolType hintType=S_UNDEFINED)
{
    if (DEBUG) std::cout << "expr" << '\n';

    // arith_op is required and defined as:
    //  relation, arith_op_pr
    // expression_pr is completely optional; defined as:
    //  either & or |, arith_op, expression_pr

    if (token() == TokenType::RS_NOT)
    {
        // not <arith_op>
        advance();
        Value val = arith_op(hintType);
        if (val.sym_type == S_INTEGER)
        {
        }
        else if (val.sym_type == S_BOOL)
        {
        }
        else
        {
            err_handler->reportError("Can only invert integers (bitwise) or bools (logical)");
        }
        // TODO: need different NOT code for ints and bools
        /*
        else
        {
            // TODO: invert val in generated code
        }
        */

        // Return becuase (not <arith_op>) can't be followed by expr_pr
        return val;
    }

    // Because arith_op is required to result in something, take its value
    //  and give it to expression_pr. expression_pr will return that value,
    //  either modified with its operation (& or |) and another arith_op, 
    //  (and optionally another expression_pr, and so on...)
    //  OR, expression_pr(val) will just return val unmodified 
    //  if there is no operator & or | as the first token.
    Value val = arith_op(hintType); 
    return expression_pr(val, hintType);
}

// lhs - left hand side of this operation. 
Value Parser::expression_pr(Value lhs, SymbolType hintType)
{
    if (DEBUG) std::cout << "expr prime" << '\n';


    if (token() == TokenType::AND
        || token() == TokenType::OR)
    {
        advance();
        Value rhs = arith_op(hintType);
        if (lhs.sym_type != rhs.sym_type)
        {
            // TODO
            // If one is a bool and one an int, convert
            //  depending on hintType
            // If hintType is neither bool nor int, assume one type.
        }
        Value val = rhs; // TODO Generate code for lhs & or | with the result of arith_op
        return expression_pr(val, hintType);
    }
    // No operation performed; return lhs unmodified.
    else return lhs;
}

Value Parser::arith_op(SymbolType hintType)
{
    if (DEBUG) std::cout << "arith op" << '\n';

    Value val = relation(hintType);
    return arith_op_pr(val, hintType);
}

Value Parser::arith_op_pr(Value lhs, SymbolType hintType)
{
    if (DEBUG) std::cout << "arith op pr" << '\n';
    // TODO: Same idea as expression_pr (and same for all other _pr fxns)
    if (token() == TokenType::PLUS || token() == TokenType::MINUS)
    {
        advance();
        Value rhs = relation(hintType); 
        //TODO
        // If one is int and one is float, 
        //  convert float one to int or int one to float,
        //  depending on which hintType is, or, 
        //  if hintType is neither, just assume conversion to int.
        // If one is neither an int nor float, error.
        return arith_op_pr(rhs, hintType);
    }
    else return lhs;
}

Value Parser::relation(SymbolType hintType)
{
    if (DEBUG) std::cout << "relation" << '\n';

    Value val = term(hintType);
    return relation_pr(val, hintType);
}

Value Parser::relation_pr(Value lhs, SymbolType hintType)
{
    if (DEBUG) std::cout << "relation pr" << '\n';
    // TODO: Same idea as expression_pr (and same for all other _pr fxns)
    if ((token() == TokenType::LT)
        | (token() == TokenType::GT)
        | (token() == TokenType::LT_EQ)
        | (token() == TokenType::GT_EQ)
        | (token() == TokenType::EQUALS)
        | (token() == TokenType::NOTEQUAL))
    {
        // TODO: Generate code. Can compare any of:
        //  Int/bool/float/char
        
        advance();
        Value val = term(hintType);
        return relation_pr(val, hintType);
    }
    else return lhs;
}

Value Parser::term(SymbolType hintType)
{
    if (DEBUG) std::cout << "term" << '\n';

    Value val = factor(hintType);
    return term_pr(val, hintType);
}

Value Parser::term_pr(Value lhs, SymbolType hintType)
{
    if (DEBUG) std::cout << "term pr" << '\n';
    // TODO: Same idea as expression_pr (and same for all other _pr fxns)

    if (token() == TokenType::MULTIPLICATION 
        || token() == TokenType::DIVISION)
    {
        //TODO
        // If one is int and one is float, 
        //  convert float one to int or int one to float,
        //  depending on which hintType is, or, 
        //  if hintType is neither, just assume conversion to int.
        // If one is neither an int nor float, error.
        advance();
        Value rhs = factor(hintType);
        Value val = rhs; // TODO: Code gen (lhs [*/] rhs)
        return term_pr(val, hintType);
    }
    else return lhs;
}

Value Parser::factor(SymbolType hintType)
{
    if (DEBUG) std::cout << "factor" << '\n';
    Value retval;

    // Token is one of:
    //  (expression), [-] name, [-] float|integer, string, char, bool 
    if (token() == TokenType::L_PAREN)
    {
        advance();
        retval = expression(hintType);
        require(TokenType::R_PAREN);
    }
    else if (token() == TokenType::MINUS)
    {
        advance();
        if (token() == TokenType::INTEGER)
        {
            retval = advance().val;
            retval.int_value = -1 * retval.int_value;
        }
        else if (token() == TokenType::FLOAT) 
        {
            retval = advance().val;
            retval.float_value = -1.0 * retval.float_value;
        }
        else if (token() == TokenType::IDENTIFIER) 
        {
            retval = name(hintType);
            // TODO Generate instructions here to mult. id. by -1?
        }
        else
        {
            std::ostringstream stream;
            stream << "Bad token following negative sign: " << TokenTypeStrings[token()];
            err_handler->reportError(stream.str());
            advance();
        }
    }
    else if (token() == TokenType::IDENTIFIER)
    {
        retval = name(hintType);
    }
    else if (curr_token.val.sym_type == S_STRING 
            || curr_token.val.sym_type == S_CHAR 
            || curr_token.val.sym_type == S_INTEGER
            || curr_token.val.sym_type == S_FLOAT
            || curr_token.val.sym_type == S_BOOL)
    {
        // Literal values
        // Consume the token and get the value
        retval = advance().val;
        // TODO: type checking 
    }
    else
    {
        std::ostringstream stream;
        stream << "Invalid token type in factor: " << TokenTypeStrings[token()];
        // Consume token and get line number 
        err_handler->reportError(stream.str(), advance().line);
    }
    
    return retval;
}

Value Parser::name(SymbolType hintType)
{
    if (DEBUG) std::cout << "name" << '\n';
    Value val;

    std::string id = require(TokenType::IDENTIFIER).val.string_value;
    SymTableEntry* entry = symtable_manager->resolve_symbol(id);
    val.sym_type = entry->sym_type;

    if (token() == TokenType::L_BRACKET)
    {
        // TODO deal with indexing 
        advance();
        expression(SymbolType::S_INTEGER);
        require(TokenType::R_BRACKET);
    }
    return val;
}

