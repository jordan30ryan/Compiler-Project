#include "parser.h"

#define P_DEBUG false

// To assist in error printing 
const char* TokenTypeStrings[] = 
{
".", ";", "(", ")", ",", "[", "]", ":", "&", "|", "+", "-", "<", ">", "<=", ">=", ":=", "==", "!=", "*", "/", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "IDENTIFIER", "UNKNOWN",
"RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT"
};

const char* SymbolTypeStrings[] = 
{
    "S_UNDEFINED", "S_STRING", "i8", "i32", "float", "i1", "S_PROCEDURE"
};

// Initialize llvm stuff

using namespace llvm;
using namespace llvm::sys;

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;

Parser::Parser(ErrHandler* handler, SymbolTableManager* manager, Scanner* scan, std::string filename)
    : err_handler(handler), symtable_manager(manager), scanner(scan)
{ 
    // Initialize curr_token so old values aren't used 
    curr_token.type = UNKNOWN;
    curr_token.val.sym_type = S_UNDEFINED;

    // TODO temporarary
    codegen_out = new std::ostringstream;

    TheModule = make_unique<Module>("my IR", TheContext);
}

Parser::~Parser() { }

TokenType Parser::token()
{
    if (curr_token.type == TokenType::FILE_END)
    {
        // Possibly desynchronized - this would go on infinitely, terminate here
        // TODO: LLVM Doesn't support exceptions
        //throw std::runtime_error("Repeated attempt to get FILE_END token. Terminating due to desynchronization.");
    }
    if (!curr_token_valid)
    {
        curr_token_valid = true;
        curr_token = scanner->getToken();
        if (P_DEBUG) std::cout << "\tGot: " << TokenTypeStrings[curr_token.type] << '\n';
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

void Parser::decl_builtins()
{
    // PUTINTEGER(i32)
    std::vector<Type*> Params(1, Type::getInt64Ty(TheContext));
    FunctionType *FT =
        FunctionType::get(Type::getVoidTy(TheContext), Params, false);
    Function *F =
        Function::Create(FT, Function::ExternalLinkage, "PUTINTEGER", TheModule.get());
/*
    *codegen_out << "declare void @PUTINTEGER(i32)" << '\n';
    *codegen_out << "declare void @PUTFLOAT(float)" << '\n';
    *codegen_out << "declare void @PUTCHAR(i8)" << '\n';
    *codegen_out << "declare void @PUTSTRING(i8*)" << '\n';
    *codegen_out << "declare void @PUTBOOL(i1)" << '\n';

    // TODO: How do these work? they get passed a variable, not returned. 
    //  Need to pass in a pointer?
    *codegen_out << "declare i32 @GETINTEGER()" << '\n';
    *codegen_out << "declare float @GETFLOAT()" << '\n';
    *codegen_out << "declare i8 @GETCHAR()" << '\n';
    *codegen_out << "declare i8* @GETSTRING()" << '\n';
    *codegen_out << "declare i1 @GETBOOL()" << '\n';
*/
}

// Get next available register number for use in LLVM
std::string Parser::next_reg() { return ""; }
/*
{
    int reg = symtable_manager->get_current_proc_next_reg(true);
    if (reg == -1) 
    {
        // Not currently in a procedure; use the general counter
        reg_no++;
        reg = reg_no;
    }
    std::ostringstream stream;
    //stream << '%' << reg;
    stream << reg;
    return stream.str();
}
*/

std::string Parser::next_label()
{
    label_no++;
    std::ostringstream stream;
    stream << "l" << label_no;
    return stream.str();
}

// Returns the literal value of a given Value as a string
std::string Parser::get_val(MyValue val) 
{
    std::ostringstream stream;

    if (val.reg != -1) 
    {
        int return_reg;
        // Value is a variable register. Access its value (not pointer to it)
        if (val.is_ptr)
        {
            *codegen_out << '\t' << next_reg() << " = load " 
                        << SymbolTypeStrings[val.sym_type]
                        << ", "
                        << SymbolTypeStrings[val.sym_type]
                        << "* %" << val.reg << '\n';
            return_reg = reg_no;
        }
        else return_reg = val.reg;

        // Return the current reg (one the var was loaded into.
        stream << '%' << return_reg; 
    }
    else
    {
        switch (val.sym_type)
        {
        case S_INTEGER:
            ConstantInt::get(TheContext, APInt(64, val.float_value));
            break;
        case S_BOOL:
            ConstantInt::get(TheContext, APInt(1, val.float_value));
            break;
        case S_FLOAT:
            ConstantFP::get(TheContext, APFloat(val.float_value));
            break;
        case S_STRING:
            // TODO
            stream << val.string_value;
            break;
        case S_CHAR:
            // Cast to int because char literals in llvm assembly 
            //  should be numbers, not the char literal itself.
            //stream << (int)val.char_value;
            ConstantInt::get(TheContext, APInt(8, (int)val.char_value));
            break;
        case S_PROCEDURE:
        case S_UNDEFINED:
            err_handler->reportError("Bad type for literal", curr_token.line);
        }
    }

    return stream.str();
}

void Parser::convert_type(MyValue& val, std::string& val_reg_str, SymbolType required_type)
{
    if (required_type == S_UNDEFINED) return;
    else if (required_type == S_INTEGER && val.sym_type == S_FLOAT)
    {
        val.sym_type = S_INTEGER;
        // If there's a register string, val is a register. Otherwise, it's a literal.
        if (val_reg_str != "")
        {
            // Generate code converting val_reg_str to a register of type float
            *codegen_out << '\t' << next_reg() << " = fptosi float " 
                << val_reg_str << " to i32" << '\n';
            // Set val to be this new converted value.
            val.reg = reg_no;
            // Set it to not be a pointer in case it was 
            val.is_ptr = false;
            val_reg_str = get_val(val); // This must return the modified value
        }
        else
        {
            // Convert literal 
            val.float_value = val.int_value;
            val.sym_type = S_FLOAT;
        }
    }
    else if (required_type == S_FLOAT && val.sym_type == S_INTEGER)
    {
        val.sym_type = S_FLOAT;
        // If there's a register string, val is a register. Otherwise, it's a literal.
        if (val_reg_str != "")
        {
            // Generate code converting val_reg_str to a temp register type int
            *codegen_out << '\t' << next_reg() << " = sitofp i32 " 
                << val_reg_str << " to float" << '\n';
            // Set val to be this new converted value.
            val.reg = reg_no;
            // Set it to not be a pointer in case it was 
            val.is_ptr = false;
            val_reg_str = get_val(val);
        }
        else 
        {
            // Convert literal 
            val.float_value = val.int_value;
            val.sym_type = S_FLOAT;
        }
    }
    else if (required_type == S_BOOL && val.sym_type == S_INTEGER)
    {
        val.sym_type = S_BOOL;
        //*codegen_out << '\t' << next_reg() << " = trunc i32 " 
            //<< val_reg_str << " to i1" << '\n';
        *codegen_out << '\t' << next_reg() << " = icmp ne i32 " 
            << val_reg_str << ", 0" << '\n';
        // Set val to be this new converted value.
        val.reg = reg_no;
        // Set it to not be a pointer in case it was 
        val.is_ptr = false;
        val_reg_str = get_val(val);
    }
    else if (required_type == S_INTEGER && val.sym_type == S_BOOL)
    {
        val.sym_type = S_INTEGER;
        *codegen_out << '\t' << next_reg() << " = zext i1 " 
            << val_reg_str << " to i32" << '\n';
        val.reg = reg_no;
        val.is_ptr = false;
        val_reg_str = get_val(val);
        // Set val to be this new converted value.
        val.reg = reg_no;
        // Set it to not be a pointer in case it was 
        val.is_ptr = false;
        val_reg_str = get_val(val);
    }
    else 
    {
        std::ostringstream stream;
        stream << "Conflicting types in conversion: " << SymbolTypeStrings[required_type] << ", " << SymbolTypeStrings[val.sym_type] << '\n';
        err_handler->reportError(stream.str(), curr_token.line);
        return;
    }
}

std::unique_ptr<llvm::Module> Parser::parse() 
{
    program();
    return std::move(TheModule);
}

void Parser::program()
{
    if (P_DEBUG) std::cout << "program" << '\n';

    // Declare builtin functions in llvm file
    decl_builtins();

    // Use main for outer program.
    // Build a simple IR
    // Set up function main (returns i32, no params)
    std::vector<Type *> Parameters;
    FunctionType *FT =
        FunctionType::get(Type::getInt32Ty(TheContext), Parameters, false);
    Function* main =
        Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    // Create basic block of main
    BasicBlock *bb = BasicBlock::Create(TheContext, "entry", main);
    Builder.SetInsertPoint(bb);

    program_header(); 
    program_body(); 
    require(TokenType::PERIOD, false);

    // Return 0 from the main function always
    Value *val = ConstantInt::get(TheContext, APInt(32, 0));
    Builder.CreateRet(val);
}

void Parser::program_header()
{
    if (P_DEBUG) std::cout << "program header" << '\n';
    require(TokenType::RS_PROGRAM);

    require(TokenType::IDENTIFIER);
    std::string program_name = curr_token.val.string_value;

    require(TokenType::RS_IS);
}

void Parser::program_body()
{
    if (P_DEBUG) std::cout << "program body" << '\n';
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
    if (P_DEBUG) std::cout << "declaration" << '\n';
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
    if (P_DEBUG) std::cout << "proc decl" << '\n';
    proc_header();
    proc_body();

    *codegen_out << "\tret void\n";
    *codegen_out << "}\n";

    // Reset to scope above this proc decl
    symtable_manager->reset_scope();

    // Add the finished procedure definition to a vector 
    //  to be added at the end of the file at the end of parsing.
    // See documentation for more info.
    procedure_defs.push_back((std::ostringstream*)codegen_out);
    // Reset codegen stream up a stream
    codegen_out = stream_stack.top();
    stream_stack.pop();
}

void Parser::proc_header()
{
    if (P_DEBUG) std::cout << "proc header" << '\n';
    require(TokenType::RS_PROCEDURE);

    // Setup symbol table so the procedure's sym table is now being used
    std::string proc_id = require(TokenType::IDENTIFIER).val.string_value;
    // Sets the current scope to this procedure's scope
    symtable_manager->set_proc_scope(proc_id);

    stream_stack.push(codegen_out);
    // Start outputting to a new stream
    codegen_out = new std::ostringstream;

    // Output floating point numbers in scientific notation.
    codegen_out->setf(std::ios_base::scientific);

    *codegen_out << "define void @" << proc_id << '(';

    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        parameter_list(); 
    require(TokenType::R_PAREN);

    std::vector<SymTableEntry*> params_vec 
        = symtable_manager->get_current_proc_params();

    for (auto it = params_vec.begin(); it != params_vec.end(); ++it)
    {
        // TODO: codegen w/ api
        /*
        // TODO pointer type? depends on whether it's out/in
        if (it != params_vec.begin()) *codegen_out << ", ";
        *codegen_out << SymbolTypeStrings[(*it)->sym_type] 
            << "* " << next_reg();
        //int entry_reg = symtable_manager->get_current_proc_next_reg(false);
        // Weird syntax; it is an iterator so it has to be derefrenced (*)
        // params_vec stores pointers to SymTableEntry so that also gets dereferenced (->)
        //(*it)->reg = entry_reg;
        */
    }

    *codegen_out << ") {\n";
}

void Parser::proc_body()
{
    if (P_DEBUG) std::cout << "proc body" << '\n';
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
    if (P_DEBUG) std::cout << "param list" << '\n';
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
    if (P_DEBUG) std::cout << "param" << '\n';

    SymTableEntry* entry = var_declaration(false, false);

    // curr_token is the typemark
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


// need_alloc - variable needs to be allocated before using (defaults to true)
//  need_alloc==false for parameter variable declarations
SymTableEntry* Parser::var_declaration(bool is_global, bool need_alloc)
{
    if (P_DEBUG) std::cout << "var decl" << '\n';
    // This is the only place in grammar type mark occurs 
    //  so it doesn't need its own function
    TokenType typemark = token();
    advance();

    std::string id = require(TokenType::IDENTIFIER).val.string_value;
    SymTableEntry* entry = symtable_manager->resolve_symbol(id, false);
    if (entry != NULL && entry->sym_type != S_UNDEFINED)
    {
        std::ostringstream stream;
        stream << "Variable " << id << " may have already been defined the local or global scope.";
        err_handler->reportError(stream.str(), curr_token.line);
    }

    std::string llvm_type;

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

        MyValue lower = lower_bound();
        require(TokenType::COLON);
        MyValue upper = upper_bound();

        require(TokenType::R_BRACKET);
        // TODO: setup the variable as an array
    }

    if (need_alloc)
    {
        if (is_global)
        {
            //TODO
        }
        else
        {
            // Allocate space for this variable and associate the 
            //  register number with the entry
            Type* type = Type::getInt64Ty(TheContext);
            Builder.CreateAlloca(type, 0, nullptr, id);
        }
    }
    return entry;
}

MyValue Parser::lower_bound()
{
    if (P_DEBUG) std::cout << "lower_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return require(TokenType::INTEGER).val;
}

MyValue Parser::upper_bound()
{
    if (P_DEBUG) std::cout << "upper_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return require(TokenType::INTEGER).val;
}

bool Parser::statement()
{
    if (P_DEBUG) std::cout << "stmnt" << '\n';

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
    if (P_DEBUG) std::cout << "identifier stmnt" << '\n';
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
    if (P_DEBUG) std::cout << "assignment stmnt" << '\n';

    // already have identifier; need to check for indexing first
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        MyValue idx = expression(S_INTEGER);
        require(TokenType::R_BRACKET);
    }
    SymTableEntry* lhs = symtable_manager->resolve_symbol(identifier); 
    // TODO: something with indexing
    require(TokenType::ASSIGNMENT);

    MyValue rhs = expression(lhs->sym_type);

    // Get the register for the expression rhs. 
    // Also load from mem if it's a variable.
    std::string rhs_reg_str = get_val(rhs);

    // Type conversion
    if (lhs->sym_type != rhs.sym_type)
    {
        convert_type(rhs, rhs_reg_str, lhs->sym_type);
    }

    /*
    // Have to do this first because get_val might generate more LLVM code
    *codegen_out << "\tstore "
        << SymbolTypeStrings[rhs.sym_type]
        << ' '
        << rhs_reg_str 
        << ", "
        << SymbolTypeStrings[rhs.sym_type]
        << "* %" 
        << lhs->reg << '\n';
    */
}

void Parser::proc_call(std::string identifier)
{
    if (P_DEBUG) std::cout << "proc call" << '\n';
    // already have identifier

    // Check symtable for the proc
    SymTableEntry* proc_entry = symtable_manager->resolve_symbol(identifier); 
    if (proc_entry == NULL || proc_entry->sym_type != S_PROCEDURE)
    {
        std::ostringstream stream;
        stream << "Procedure " << identifier << " not defined\n";
        err_handler->reportError(stream.str(), curr_token.line);
    }

    std::vector<MyValue> arg_list;
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        arg_list = argument_list(proc_entry);
    require(TokenType::R_PAREN);
    
    std::vector<std::string> arg_list_strings;
    for (auto it : arg_list)
    {
        std::ostringstream stringbuilder;
        stringbuilder 
            << SymbolTypeStrings[it.sym_type]
            << ' '
            << get_val(it);
        arg_list_strings.push_back(stringbuilder.str());
    }

    *codegen_out << "\tcall void @" << identifier << "(";
    for (auto it = arg_list_strings.begin(); it != arg_list_strings.end(); ++it)
    {
        if (it != arg_list_strings.begin())
            *codegen_out << ", ";
        *codegen_out << *it;
    }
    *codegen_out << ")\n";
}

std::vector<MyValue> Parser::argument_list(SymTableEntry* proc_entry)
{
    if (P_DEBUG) std::cout << "arg list" << '\n';
    std::vector <MyValue> vec;
    for (auto param : proc_entry->parameters)
    {
        MyValue val = expression(param->sym_type);
        // Expression should do type conversion.
        // If it's not the right type now, it probably can't be converted.
        if (val.sym_type != param->sym_type)
        {
            std::ostringstream stream;
            stream 
                << "Procedure call paramater type doesn't match expected type."
                << "\n\tGot:\t\t" << SymbolTypeStrings[val.sym_type] 
                << "\n\tExpected:\t" << SymbolTypeStrings[param->sym_type];
            err_handler->reportError(stream.str(), curr_token.line);
        }

        vec.push_back(val); 

        if (token() == TokenType::COMMA) 
        {
            advance();
            continue;
        }
        else break;
    }
    return vec;
}

void Parser::if_statement()
{
    if (P_DEBUG) std::cout << "if" << '\n';
    require(TokenType::RS_IF);

    require(TokenType::L_PAREN);
    MyValue condition = expression(S_BOOL);
    require(TokenType::R_PAREN);

    require(TokenType::RS_THEN);

    std::string condition_reg = get_val(condition);

    std::string then_label = next_label();
    std::string else_label = next_label();
    std::string after_label = next_label();

    *codegen_out << '\t' << "br i1 " << condition_reg 
        << ", label %" << then_label << ", label %" << else_label << '\n';


    *codegen_out << then_label << ": \n"; // begin then block

    bool first_stmnt = true;
    while (true)
    {
        bool valid = statement();
        // Make sure there is at least one valid statement
        if (first_stmnt && !valid)
        {
            err_handler->reportError("No statement in IF body", curr_token.line);
        }
        else require(TokenType::SEMICOLON);
        first_stmnt = false;

        if (token() == TokenType::RS_END) 
        {
            *codegen_out << '\t' << "br label %" << after_label << '\n';
            break;
        }
        if (token() == TokenType::RS_ELSE) 
        {
            *codegen_out << '\t' << "br label %" << after_label << '\n';
            *codegen_out << else_label << ": \n"; // begin else block
            advance();
            continue;
        }
    }

    *codegen_out << after_label << ": \n"; // begin after block
    
    require(TokenType::RS_END);
    require(TokenType::RS_IF);
}

void Parser::loop_statement()
{
    if (P_DEBUG) std::cout << "for" << '\n';
    require(TokenType::RS_FOR);

    require(TokenType::L_PAREN);
    require(TokenType::IDENTIFIER);
    assignment_statement(curr_token.val.string_value); 
    require(TokenType::SEMICOLON);

    std::string start_loop_label = next_label();
    std::string loop_stmnts_label = next_label();
    std::string after_loop_label = next_label();

    // Need to explicitly break from a basic block.
    *codegen_out << "\tbr label %" << start_loop_label << '\n'; 

    *codegen_out << start_loop_label << ": \n"; // begin for block (expr check)

    // Generate expression code in for block
    MyValue condition = expression(S_BOOL);
    require(TokenType::R_PAREN);

    std::string condition_reg = get_val(condition);

    *codegen_out << "\tbr i1 " << condition_reg 
        << ", label %" << loop_stmnts_label 
        << ", label %" << after_loop_label << '\n';

    *codegen_out << loop_stmnts_label << ": \n"; // begin for statements block

    // Consume and generate code for all inner for statements
    while (true)
    {
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
    }

    // End of for statements; jmp to beginning of for
    *codegen_out << "\tbr label %" << start_loop_label << '\n';
    
    *codegen_out << after_loop_label << ": \n"; // begin block after for

    require(TokenType::RS_END); // Just to be sure, also to advance the token
    require(TokenType::RS_FOR);
}

void Parser::return_statement()
{
    if (P_DEBUG) std::cout << "return" << '\n';
    require(TokenType::RS_RETURN);
}

// hintType - the expected type (e.g. if this is an assignment)
//  used as a hint to expression on what type to convert to 
//  (if type conversion is needed)
MyValue Parser::expression(SymbolType hintType=S_UNDEFINED)
{
    if (P_DEBUG) std::cout << "expr" << '\n';

    MyValue retval;

    // arith_op is required and defined as:
    //  relation, arith_op_pr
    // expression_pr is completely optional; defined as:
    //  either & or |, arith_op, expression_pr

    if (token() == TokenType::RS_NOT)
    {
        // not <arith_op>
        advance();
        MyValue val = arith_op(hintType);
        std::string valstr = get_val(val);
        if (val.sym_type == S_INTEGER)
        {
            *codegen_out << '\t' << next_reg() << " = xor i32 " 
                << valstr << ", -1" << '\n';
            val.reg = reg_no;
            val.is_ptr = false;
        }
        else if (val.sym_type == S_BOOL)
        {
            *codegen_out << '\t' << next_reg() << " = xor i1 " 
                << valstr << ", 1" << '\n';
            val.reg = reg_no;
            val.is_ptr = false;
        }
        else
        {
            err_handler->reportError("Can only invert integers (bitwise) or bools (logical)", curr_token.line);
        }

        // Return becuase (not <arith_op>) is a complete expression
        retval = val;
    }
    else 
    {
        // Because arith_op is required to result in something, take its value
        //  and give it to expression_pr. expression_pr will return that value,
        //  either modified with its operation (& or |) and another arith_op, 
        //  (and optionally another expression_pr, and so on...)
        //  OR, expression_pr(val) will just return val unmodified 
        //  if there is no operator & or | as the first token.
        MyValue val = arith_op(hintType); 
        retval = expression_pr(val, hintType);
    }

    // Type conversion to expected type before returning from expression.
    if (hintType != retval.sym_type)
    {
        std::string retval_str = get_val(retval);
        convert_type(retval, retval_str, hintType);
    }

    return retval;
}

// lhs - left hand side of this operation. 
MyValue Parser::expression_pr(MyValue lhs, SymbolType hintType)
{
    if (P_DEBUG) std::cout << "expr prime" << '\n';


    if (token() == TokenType::AND
        || token() == TokenType::OR)
    {
        TokenType op = advance().type;

        MyValue rhs = arith_op(hintType);

        // Need to get value string here because it might requrie 
        //  code gen (for loading from variable pointers)
        std::string lhs_str = get_val(lhs);
        std::string rhs_str = get_val(rhs);

        // If one is a bool and one an int, convert
        if (lhs.sym_type == S_BOOL && rhs.sym_type == S_INTEGER)
        {
            if (hintType == S_BOOL)
                convert_type(rhs, rhs_str, S_BOOL);
            else 
                convert_type(lhs, lhs_str, S_INTEGER);
        }
        else if (lhs.sym_type == S_INTEGER && rhs.sym_type == S_BOOL)
        {
            if (hintType == S_BOOL)
                convert_type(lhs, lhs_str, S_BOOL);
            else 
                convert_type(rhs, lhs_str, S_INTEGER);
        }
        else if (lhs.sym_type != rhs.sym_type 
                    && lhs.sym_type != S_BOOL 
                    && lhs.sym_type != S_INTEGER)
        {
            // Types aren't the same or aren't both bool/int
            err_handler->reportError("Bitwise or boolean operations are only defined on bool and integer types", curr_token.line);
        }

        *codegen_out << '\t' << next_reg() << " = "
            << (op == TokenType::AND ? "and" : "or") 
            << ' '
            << SymbolTypeStrings[lhs.sym_type]
            << ' '
            << lhs_str
            << ", "
            << rhs_str
            << '\n';

        MyValue result;
        result.reg = reg_no;
        result.sym_type = lhs.sym_type;

        return expression_pr(result, hintType);
    }

    // No operation performed; return lhs unmodified.
    else return lhs;
}

MyValue Parser::arith_op(SymbolType hintType)
{
    if (P_DEBUG) std::cout << "arith op" << '\n';

    MyValue val = relation(hintType);
    return arith_op_pr(val, hintType);
}

MyValue Parser::arith_op_pr(MyValue lhs, SymbolType hintType)
{
    if (P_DEBUG) std::cout << "arith op pr" << '\n';

    if (token() == TokenType::PLUS || token() == TokenType::MINUS)
    {
        // Advance and save current token's operator.
        TokenType op = advance().type;

        MyValue rhs = relation(hintType); 

        // Need to get value string here because it might requrie 
        //  code gen (for loading from variable pointers)
        std::string lhs_str = get_val(lhs);
        std::string rhs_str = get_val(rhs);

        // If one is int and one is float, 
        //  convert all to float to get the most precision.
        // Any other types can't be used here.
        if (lhs.sym_type == S_INTEGER && rhs.sym_type == S_FLOAT)
        {
            // Convert lhs to float
            convert_type(lhs, lhs_str, S_FLOAT);
        }
        else if (lhs.sym_type == S_FLOAT && rhs.sym_type == S_INTEGER)
        {
            // Convert rhs to float
            convert_type(rhs, rhs_str, S_FLOAT);
        }
        else if (lhs.sym_type != rhs.sym_type 
                    && lhs.sym_type != S_FLOAT
                    && lhs.sym_type != S_INTEGER)
        {
            // Types aren't the same or aren't both float/int
            err_handler->reportError("Arithmetic operations are only defined on float and integer types", curr_token.line);
            // TODO: Return?
        }

        *codegen_out << '\t' << next_reg() << " = "
            << (op == TokenType::PLUS 
                    ?  lhs.sym_type == S_FLOAT ? "fadd" : "add" 
                    : lhs.sym_type == S_FLOAT ? "fsub" : "sub") 
            << ' '
            << SymbolTypeStrings[lhs.sym_type]
            << ' '
            << lhs_str
            << ", "
            << rhs_str
            << '\n';

        MyValue result;
        result.reg = reg_no;
        result.sym_type = lhs.sym_type;

        return arith_op_pr(result, hintType);
    }
    else return lhs;
}

MyValue Parser::relation(SymbolType hintType)
{
    if (P_DEBUG) std::cout << "relation" << '\n';

    MyValue val = term(hintType);
    return relation_pr(val, hintType);
}

MyValue Parser::relation_pr(MyValue lhs, SymbolType hintType)
{
    if (P_DEBUG) std::cout << "relation pr" << '\n';

    if ((token() == TokenType::LT)
        | (token() == TokenType::GT)
        | (token() == TokenType::LT_EQ)
        | (token() == TokenType::GT_EQ)
        | (token() == TokenType::EQUALS)
        | (token() == TokenType::NOTEQUAL))
    {
        TokenType op = advance().type;
        MyValue rhs = term(hintType);

        // Need to get value string here because it might requrie 
        //  code gen (for loading from variable pointers)
        std::string lhs_str = get_val(lhs);
        std::string rhs_str = get_val(rhs);

        // Type conversion
        if (lhs.sym_type == S_STRING || rhs.sym_type == S_STRING)
        {
            err_handler->reportError("Relation operators are not defined for strings.", curr_token.line);
        }
        if (lhs.sym_type != rhs.sym_type)
        {
            if (lhs.sym_type == S_FLOAT && rhs.sym_type == S_INTEGER)
            {
                // Always convert up to float to avoid losing information
                convert_type(rhs, rhs_str, S_FLOAT);
            }
            else if (lhs.sym_type == S_INTEGER && rhs.sym_type == S_FLOAT)
            {
                // Always convert up to float to avoid losing information
                convert_type(lhs, lhs_str, S_FLOAT);
            }
            else if (lhs.sym_type == S_BOOL && rhs.sym_type == S_INTEGER)
            {
                // Prefer conversion to hint type if possible to simplify later
                if (hintType == S_BOOL) convert_type(rhs, rhs_str, S_BOOL);
                else convert_type(lhs, lhs_str, S_INTEGER);
            }
            else if (lhs.sym_type == S_INTEGER && rhs.sym_type == S_BOOL)
            {
                // Prefer conversion to hint type if possible to simplify later
                if (hintType == S_BOOL) convert_type(lhs, lhs_str, S_BOOL);
                else convert_type(rhs, rhs_str, S_INTEGER);
            }
        }

        std::string llvm_op;
        switch (op)
        {
            // TODO: ordered? not ordered? signed? unsigned?
            case LT:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "olt" : "slt";
                break;
            case GT:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "ogt" : "sgt";
                break;
            case LT_EQ:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "ole" : "sle";
                break;
            case GT_EQ:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "oge" : "sge";
                break;
            case EQUALS:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "oeq" : "eq";
                break;
            case NOTEQUAL:
                llvm_op = 
                    lhs.sym_type == S_FLOAT ? "one" : "ne";
                break;
            default:
                // This shouldn't happen; how'd we get in this outer if anyway lol
                break;
        }

        *codegen_out << '\t' << next_reg() << " = "
            // Type checking above will convert it to either a float or int-like type
            << (lhs.sym_type == S_FLOAT ? "fcmp" : "icmp")
            << ' '
            << llvm_op
            << ' '
            << SymbolTypeStrings[lhs.sym_type]
            << ' '
            << lhs_str
            << ", "
            << rhs_str
            << '\n';

        MyValue result;
        result.reg = reg_no;
        // All relational compares return a bool
        result.sym_type = S_BOOL;

        return relation_pr(result, hintType);
    }
    else return lhs;
}

MyValue Parser::term(SymbolType hintType)
{
    if (P_DEBUG) std::cout << "term" << '\n';

    MyValue val = factor(hintType);
    return term_pr(val, hintType);
}

// Multiplication / Division 
MyValue Parser::term_pr(MyValue lhs, SymbolType hintType)
{
    if (P_DEBUG) std::cout << "term pr" << '\n';


    if (token() == TokenType::MULTIPLICATION 
        || token() == TokenType::DIVISION)
    {
        TokenType op = advance().type;
        MyValue rhs = factor(hintType);

        // Need to get value string here because it might requrie 
        //  code gen (for loading from variable pointers)
        std::string lhs_str = get_val(lhs);
        std::string rhs_str = get_val(rhs);

        if (lhs.sym_type == S_INTEGER && rhs.sym_type == S_FLOAT)
        {
            // Convert lhs to float
            convert_type(lhs, lhs_str, S_FLOAT);
        }
        else if (lhs.sym_type == S_FLOAT && rhs.sym_type == S_INTEGER)
        {
            // Convert rhs to float
            convert_type(rhs, rhs_str, S_FLOAT);
        }
        else if (lhs.sym_type != rhs.sym_type 
                    && lhs.sym_type != S_FLOAT
                    && lhs.sym_type != S_INTEGER)
        {
            // Types aren't the same or aren't both float/int
            err_handler->reportError("Term operations (multiplication and division) are only defined on float and integer types.", curr_token.line);
        }

        *codegen_out << '\t' << next_reg() << " = "
            // Multiplication
            << (op == TokenType::MULTIPLICATION 
                // Multiplication
                // fmul (floating) / mul (ints)
                ? lhs.sym_type == S_FLOAT ? "fmul" : "mul"
                // Division
                // fdiv (floating) / udiv (unsigned int)
                : lhs.sym_type == S_FLOAT ? "fdiv" : "sdiv") 
            << ' '
            << SymbolTypeStrings[lhs.sym_type]
            << ' '
            << lhs_str
            << ", "
            << rhs_str
            << '\n';

        MyValue result;
        result.reg = reg_no;
        result.sym_type = lhs.sym_type;

        return term_pr(result, hintType);
    }
    else return lhs;
}

MyValue Parser::factor(SymbolType hintType)
{
    if (P_DEBUG) std::cout << "factor" << '\n';
    MyValue retval;

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
            err_handler->reportError(stream.str(), curr_token.line);
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

MyValue Parser::name(SymbolType hintType)
{
    if (P_DEBUG) std::cout << "name" << '\n';
    MyValue val;

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

    //val.reg = entry->reg;
    val.is_ptr = true;
    return val;
}

