#include "parser.h"

#define P_DEBUG false

// To assist in error printing 
const char* TokenTypeStrings[] = 
{
".", ";", "(", ")", ",", "[", "]", ":", "&", "|", "+", "-", "<", ">", "<=", ">=", ":=", "==", "!=", "*", "/", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "IDENTIFIER", "UNKNOWN",
"RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT"
};

/*
const char* SymbolTypeStrings[] = 
{
    "S_UNDEFINED", "S_STRING", "i8", "i32", "float", "i1", "S_PROCEDURE"
};
*/

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

void Parser::decl_single_builtin(std::string name, Type* paramtype)
{
    std::vector<Type*> Params(1, paramtype);
    FunctionType *FT =
        FunctionType::get(Type::getVoidTy(TheContext), Params, false);
    Function *F =
        Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());

    // Associate the LLVM function we created with its symboltable entry
    SymTableEntry* entry = symtable_manager->resolve_symbol(name, true);
    entry->function = F;
}

void Parser::decl_builtins()
{
    decl_single_builtin("PUTINTEGER", Type::getInt32PtrTy(TheContext));
    decl_single_builtin("PUTFLOAT", Type::getFloatPtrTy(TheContext));
    decl_single_builtin("PUTCHAR", Type::getInt8PtrTy(TheContext));
    decl_single_builtin("PUTSTRING", Type::getInt32PtrTy(TheContext));
    decl_single_builtin("PUTBOOL", Type::getInt1PtrTy(TheContext));

    decl_single_builtin("GETINTEGER", Type::getInt32PtrTy(TheContext));
    decl_single_builtin("GETFLOAT", Type::getFloatPtrTy(TheContext));
    decl_single_builtin("GETCHAR", Type::getInt8PtrTy(TheContext));
    decl_single_builtin("GETSTRING", Type::getInt32PtrTy(TheContext));
    decl_single_builtin("GETBOOL", Type::getInt1PtrTy(TheContext));
}

// Get next available register number for use in LLVM
//std::string Parser::next_reg() { return ""; }

Value* Parser::convert_type(Value* val, Type* required_type)
{
    Value* retval;

    if (required_type == val->getType() || required_type == nullptr) return nullptr;

    if (required_type == Type::getInt32Ty(TheContext)
             && val->getType() == Type::getFloatTy(TheContext))
    {
        retval = Builder.CreateFPToSI(val, required_type);
    }
    else if (required_type == Type::getFloatTy(TheContext)
            && val->getType() == Type::getInt32Ty(TheContext))
    {
        retval = Builder.CreateSIToFP(val, required_type);
    }
    else if (required_type == Type::getInt1Ty(TheContext) 
            && val->getType() == Type::getInt32Ty(TheContext))
    {
        // Compare to 0 to convert to bool
        retval = Builder.CreateICmpNE(val, 
                ConstantInt::get(TheContext, APInt(1, 0)));
    }
    else if (required_type == Type::getInt32Ty(TheContext) 
            && val->getType() == Type::getInt1Ty(TheContext))
    {
        retval = Builder.CreateZExt(val, required_type);
    }
    else 
    {
        TheModule->print(llvm::errs(), nullptr);

        err_handler->reportError("Conflicting types in conversion: req'd:\n", curr_token.line);
        required_type->print(llvm::errs(), nullptr);
        err_handler->reportError("got: \n", curr_token.line);
        val->getType()->print(llvm::errs(), nullptr);
        return nullptr;
    }

    return retval;
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

    Builder.CreateRetVoid();

    // Reset to scope above this proc decl
    symtable_manager->reset_scope();

    // Get previous IP from proc manager
    //  and restore it so the builder appends to it again
    Builder.restoreIP(symtable_manager->get_insert_point());
}

void Parser::proc_header()
{
    if (P_DEBUG) std::cout << "proc header" << '\n';
    require(TokenType::RS_PROCEDURE);

    // Setup symbol table so the procedure's sym table is now being used
    std::string proc_id = require(TokenType::IDENTIFIER).val.string_value;

    IRBuilderBase::InsertPoint ip = Builder.saveIP();
    symtable_manager->save_insert_point(ip);

    // Sets the current scope to this procedure's scope
    symtable_manager->set_proc_scope(proc_id);

    // Parse params
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        parameter_list(); 
    require(TokenType::R_PAREN);

    std::vector<SymTableEntry*> params_vec 
        = symtable_manager->get_current_proc_params();

    std::vector<Type *> parameters;

    // TODO: Handle out/inout types
    for (auto param : params_vec)
    {
        Type* param_type;
        switch (param->sym_type)
        {
        case S_INTEGER:
            param_type = Type::getInt32PtrTy(TheContext);
            break;
        case S_FLOAT:
            param_type = Type::getFloatPtrTy(TheContext);
            break;
        case S_STRING:
            //TODO
            param_type = Type::getInt8PtrTy(TheContext);
            break;
        case S_CHAR:
            param_type = Type::getInt8PtrTy(TheContext);
            break;
        case S_BOOL:
            param_type = Type::getInt1PtrTy(TheContext);
            break;
        default:
            // TODO: Err 
            break;
        }
        parameters.push_back(param_type);
    }

    FunctionType *FT =
        FunctionType::get(Type::getVoidTy(TheContext), parameters, false);

    Function* F = Function::Create(FT, 
        Function::ExternalLinkage, 
        proc_id, 
        TheModule.get());

    verifyFunction(*F);

    // Set arg names to their real ids
    int k = 0;
    for (auto &arg : F->args())
    {
        params_vec[k]->value = &arg;
        arg.setName(params_vec[k++]->id);
    }

    symtable_manager->set_curr_proc_function(F);

    BasicBlock *bb = BasicBlock::Create(TheContext, "entry", F);

    // Set IP to this function's basic block
    Builder.SetInsertPoint(bb);
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

    // The llvm type to allocate for this variable
    Type* allocation_type;

    // For initializing globals, if applicable
    Constant* global_init_constant;

    // entry sym_type is reduntant with entry's LLVM value
    switch (typemark)
    {
    case RS_STRING:
        entry->sym_type = S_STRING;
        break;
    case RS_CHAR:
        entry->sym_type = S_CHAR;
        allocation_type = Type::getInt8Ty(TheContext);
        if (is_global) 
            global_init_constant 
                = ConstantInt::get(TheContext, APInt(8, 0));
        break;
    case RS_INTEGER:
        entry->sym_type = S_INTEGER;
        allocation_type = Type::getInt32Ty(TheContext);
        if (is_global) 
            global_init_constant 
                = ConstantInt::get(TheContext, APInt(32, 0));
        break;
    case RS_FLOAT:
        entry->sym_type = S_FLOAT;
        allocation_type = Type::getFloatTy(TheContext);
        if (is_global) 
            global_init_constant 
                = ConstantFP::get(TheContext, APFloat(0.));
        break;
    case RS_BOOL:
        entry->sym_type = S_BOOL;
        allocation_type = Type::getInt1Ty(TheContext);
        if (is_global) 
            global_init_constant 
                = ConstantInt::get(TheContext, APInt(1, 0));
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

        Value* lower = lower_bound();
        require(TokenType::COLON);
        Value* upper = upper_bound();

        require(TokenType::R_BRACKET);
        // TODO: setup the variable as an array
    }

    if (need_alloc)
    {
        if (is_global)
        {
            GlobalVariable* global = new GlobalVariable(*TheModule, 
                allocation_type, 
                false,
                GlobalValue::ExternalLinkage,
                global_init_constant,
                id,
                nullptr,
                GlobalValue::ThreadLocalMode::LocalDynamicTLSModel);
            entry->value = global;

        }
        else
        {
            // Allocate space for this variable 
            entry->value 
                = Builder.CreateAlloca(allocation_type, 0, nullptr, id);
        }
    }
    return entry;
}

Value* Parser::lower_bound()
{
    if (P_DEBUG) std::cout << "lower_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return ConstantInt::get(TheContext, 
        APInt(32, require(TokenType::INTEGER).val.int_value));
}

Value* Parser::upper_bound()
{
    if (P_DEBUG) std::cout << "upper_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    return ConstantInt::get(TheContext, 
        APInt(32, require(TokenType::INTEGER).val.int_value));
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
        // TODO: something with indexing
        advance();
        Value* idx = expression(Type::getInt32Ty(TheContext));
        require(TokenType::R_BRACKET);
    }

    // RS_OUT - we want to write to this variable
    SymTableEntry* entry = symtable_manager->resolve_symbol(identifier, true, RS_OUT); 
    Value* lhs = entry->value;

    require(TokenType::ASSIGNMENT);

    Type* lhs_stored_type = 
        cast<PointerType>(lhs->getType())->getElementType();
    Value* rhs = expression(lhs_stored_type);

    /* this isn't needed right?
    // Type conversion
    if (lhs_stored_type != rhs->getType())
    {
        rhs = convert_type(rhs, lhs_stored_type);
    }
    */

    // Store rhs into lhs(ptr)
    Builder.CreateStore(rhs, lhs);
}

void Parser::proc_call(std::string identifier)
{
    if (P_DEBUG) std::cout << "proc call" << '\n';
    // already have identifier

    // Check symtable for the proc
    SymTableEntry* proc_entry = symtable_manager->resolve_symbol(identifier, true); 
    if (proc_entry == NULL || proc_entry->sym_type != S_PROCEDURE)
    {
        std::ostringstream stream;
        stream << "Procedure " << identifier << " not defined\n";
        err_handler->reportError(stream.str(), curr_token.line);
        return;
    }


    std::vector<Value*> arg_list;
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        arg_list = argument_list(proc_entry);
    require(TokenType::R_PAREN);

    Builder.CreateCall(proc_entry->function, arg_list);
}

std::vector<Value*> Parser::argument_list(SymTableEntry* proc_entry)
{
    if (P_DEBUG) std::cout << "arg list" << '\n';

    std::vector<Value*> vec;

    Function* f = proc_entry->function;
    for (auto& parm : f->args())
    {
        AllocaInst* valptr;
        // If it's just a name, return a pointer to the var (don't load)
        if (token() == TokenType::IDENTIFIER)
            valptr = cast<AllocaInst>(name(parm.getType(), false));
        // Otherwise, parse an expression then convert to a pointer
        else
        {
            // All parameters are pointers, so this is necessary 
            // Params need to be pointers for pass by ref (out or inout)
            //  but expressions are never pointers, so we need to get
            //  a pointer to the value the expression returns then we
            //  can pass that into the function call
            Type* realType = cast<PointerType>(parm.getType())->getElementType();
            //realType->print(llvm::errs(), nullptr);
            //parm.print(llvm::errs(), nullptr);
            // TODO: realtype is void here for some reason
            Value* val = expression(realType);
            valptr = Builder.CreateAlloca(realType);
            // Store val into valptr
            Builder.CreateStore(val, valptr);
        }

        // Expression should do type conversion.
        // If it's not the right type now, it probably can't be converted.
        if (parm.getType() != valptr->getType())
        {
            err_handler->reportError("Procedure call paramater type doesn't match expected type. \n\tGot:\t\t", curr_token.line);
            parm.getType()->print(llvm::errs(), nullptr);

            err_handler->reportError("\n\tExpected:\t", curr_token.line);
            valptr->getType()->print(llvm::errs(), nullptr);
        }

        vec.push_back(valptr); 

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
    Value* condition = expression(Type::getInt1Ty(TheContext));
    require(TokenType::R_PAREN);

    require(TokenType::RS_THEN);

    //std::string condition_reg = get_val(condition);

    //std::string then_label = next_label();
    //std::string else_label = next_label();
    //std::string after_label = next_label();

/*
    *codegen_out << '\t' << "br i1 " << condition_reg 
        << ", label %" << then_label << ", label %" << else_label << '\n';
*/


    //*codegen_out << then_label << ": \n"; // begin then block

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
            //*codegen_out << '\t' << "br label %" << after_label << '\n';
            break;
        }
        if (token() == TokenType::RS_ELSE) 
        {
            //*codegen_out << '\t' << "br label %" << after_label << '\n';
            //*codegen_out << else_label << ": \n"; // begin else block
            advance();
            continue;
        }
    }

    //*codegen_out << after_label << ": \n"; // begin after block
    
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

    //std::string start_loop_label = next_label();
    //std::string loop_stmnts_label = next_label();
    //std::string after_loop_label = next_label();

    // Need to explicitly break from a basic block.
    //*codegen_out << "\tbr label %" << start_loop_label << '\n'; 

    //*codegen_out << start_loop_label << ": \n"; // begin for block (expr check)

    // Generate expression code in for block
    Value* condition = expression(Type::getInt1Ty(TheContext));
    require(TokenType::R_PAREN);

    //std::string condition_reg = get_val(condition);

    /*
    *codegen_out << "\tbr i1 " << condition_reg 
        << ", label %" << loop_stmnts_label 
        << ", label %" << after_loop_label << '\n';
        */

    //*codegen_out << loop_stmnts_label << ": \n"; // begin for statements block

    // Consume and generate code for all inner for statements
    while (true)
    {
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
    }

    // End of for statements; jmp to beginning of for
    //*codegen_out << "\tbr label %" << start_loop_label << '\n';
    
    //*codegen_out << after_loop_label << ": \n"; // begin block after for

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
Value* Parser::expression(Type* hintType)
{
    if (P_DEBUG) std::cout << "expr" << '\n';

    Value* retval;

    // arith_op is required and defined as:
    //  relation, arith_op_pr
    // expression_pr is completely optional; defined as:
    //  either & or |, arith_op, expression_pr

    if (token() == TokenType::RS_NOT)
    {
        // not <arith_op>
        advance();
        Value* val = arith_op(hintType);
        if (val->getType()->isIntegerTy(32))
        {
            retval = Builder.CreateXor(val, ConstantInt::get(TheContext, APInt(32, -1)));
        }
        if (val->getType()->isIntegerTy(1))
        {
            retval = Builder.CreateXor(val, ConstantInt::get(TheContext, APInt(1, 1)));
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
        Value* val = arith_op(hintType); 
        retval = expression_pr(val, hintType);
    }

    // Type conversion to expected type before returning from expression.
    if (hintType != retval->getType())
    {
        retval = convert_type(retval, hintType);
    }

    return retval;
}

// lhs - left hand side of this operation. 
Value* Parser::expression_pr(Value* lhs, Type* hintType)
{
    if (P_DEBUG) std::cout << "expr prime" << '\n';


    if (token() == TokenType::AND
        || token() == TokenType::OR)
    {
        TokenType op = advance().type;

        Value* rhs = arith_op(hintType);

        // If one is a bool and one an int, convert
        if (lhs->getType() == Type::getInt1Ty(TheContext) 
            && rhs->getType() == Type::getInt32Ty(TheContext))
        {
            if (hintType == Type::getInt1Ty(TheContext))
                rhs = convert_type(rhs, Type::getInt1Ty(TheContext));
            else 
                lhs = convert_type(lhs, Type::getInt32Ty(TheContext));
        }
        else if (lhs->getType() == Type::getInt32Ty(TheContext) 
            && rhs->getType() == Type::getInt1Ty(TheContext))
        {
            if (hintType == Type::getInt1Ty(TheContext))
                lhs = convert_type(lhs, Type::getInt1Ty(TheContext));
            else 
                rhs = convert_type(rhs, Type::getInt32Ty(TheContext));
        }
        else if (lhs->getType() != rhs->getType()
                    && lhs->getType() != Type::getInt1Ty(TheContext) 
                    && lhs->getType() != Type::getInt32Ty(TheContext))
        {
            // Types aren't the same or aren't both bool/int
            err_handler->reportError("Bitwise or boolean operations are only defined on bool and integer types", curr_token.line);
        }

        Value* result;
        if (op == TokenType::AND)
            result = Builder.CreateAnd(lhs, rhs);
        else
            result = Builder.CreateOr(lhs, rhs);

        return expression_pr(result, hintType);
    }

    // No operation performed; return lhs unmodified.
    else return lhs;
}

Value* Parser::arith_op(Type* hintType)
{
    if (P_DEBUG) std::cout << "arith op" << '\n';

    Value* val = relation(hintType);
    return arith_op_pr(val, hintType);
}

Value* Parser::arith_op_pr(Value* lhs, Type* hintType)
{
    if (P_DEBUG) std::cout << "arith op pr" << '\n';

    if (token() == TokenType::PLUS || token() == TokenType::MINUS)
    {
        // Advance and save current token's operator.
        TokenType op = advance().type;

        Value* rhs = relation(hintType); 

        // Type conversion

        // If one is int and one is float, 
        //  convert all to float to get the most precision.
        // Any other types can't be used here.
        if (lhs->getType() == Type::getInt32Ty(TheContext) 
            && rhs->getType() == Type::getFloatTy(TheContext))
        {
            // Convert lhs to float
            lhs = convert_type(lhs, Type::getFloatTy(TheContext));
        }
        else if (lhs->getType() == Type::getFloatTy(TheContext) 
            && rhs->getType() == Type::getInt32Ty(TheContext))
        {
            // Convert rhs to float
            rhs = convert_type(rhs, Type::getFloatTy(TheContext));
        }
        else if (lhs->getType() != rhs->getType() 
                    && lhs->getType() != Type::getFloatTy(TheContext)
                    && lhs->getType() != Type::getInt32Ty(TheContext))
        {
            // Types aren't the same or aren't both float/int
            err_handler->reportError("Arithmetic operations are only defined on float and integer types", curr_token.line);
            // TODO: Return?
        }

        Value* result;
        if (op == TokenType::PLUS)
        {
            if (lhs->getType()->isFloatTy())
                result = Builder.CreateFAdd(lhs, rhs);
            else
                result = Builder.CreateAdd(lhs, rhs);
        }
        else
        {
            if (lhs->getType()->isFloatTy())
                result = Builder.CreateFSub(lhs, rhs);
            else
                result = Builder.CreateSub(lhs, rhs);
        }

        return arith_op_pr(result, hintType);
    }
    else return lhs;
}

Value* Parser::relation(Type* hintType)
{
    if (P_DEBUG) std::cout << "relation" << '\n';

    Value* val = term(hintType);
    return relation_pr(val, hintType);
}

Value* Parser::relation_pr(Value* lhs, Type* hintType)
{
    if (P_DEBUG) std::cout << "relation pr" << '\n';

    if ((token() == TokenType::LT)
        || (token() == TokenType::GT)
        || (token() == TokenType::LT_EQ)
        || (token() == TokenType::GT_EQ)
        || (token() == TokenType::EQUALS)
        || (token() == TokenType::NOTEQUAL))
    {
        TokenType op = advance().type;
        Value* rhs = term(hintType);

        // Type conversion
        /*
        // TODO: Strings
        if (lhs->getType() == S_STRING || rhs->getType() == S_STRING)
        {
            err_handler->reportError("Relation operators are not defined for strings.", curr_token.line);
        }
        */
        if (lhs->getType() != rhs->getType())
        {
            if (lhs->getType() == Type::getFloatTy(TheContext) 
                && rhs->getType() == Type::getInt32Ty(TheContext))
            {
                // Always convert up to float to avoid losing information
                convert_type(rhs, Type::getFloatTy(TheContext));
            }
            else if (lhs->getType() == Type::getInt32Ty(TheContext) 
                && rhs->getType() == Type::getFloatTy(TheContext))
            {
                // Always convert up to float to avoid losing information
                convert_type(lhs, Type::getFloatTy(TheContext));
            }
            else if (lhs->getType() == Type::getInt1Ty(TheContext)
                && rhs->getType() == Type::getInt32Ty(TheContext))
            {
                // Prefer conversion to hint type if possible to simplify later
                if (hintType == Type::getInt1Ty(TheContext)) 
                    convert_type(rhs, Type::getInt1Ty(TheContext));
                else 
                    convert_type(lhs, Type::getInt32Ty(TheContext));
            }
            else if (lhs->getType() == Type::getInt32Ty(TheContext)     
                && rhs->getType() == Type::getInt1Ty(TheContext))
            {
                // Prefer conversion to hint type if possible to simplify later
                if (hintType == Type::getInt1Ty(TheContext)) 
                    convert_type(lhs, Type::getInt1Ty(TheContext));
                else 
                    convert_type(rhs, Type::getInt32Ty(TheContext));
            }
        }

        Value* result;
        switch (op)
        {
            // TODO: ordered? not ordered? signed? unsigned?
            case LT:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpOLE(lhs, rhs);
                else
                    result = Builder.CreateICmpSLT(lhs, rhs);
                break;
            case GT:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpOGT(lhs, rhs);
                else
                    result = Builder.CreateICmpSGT(lhs, rhs);
                break;
            case LT_EQ:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpOLE(lhs, rhs);
                else
                    result = Builder.CreateICmpSLE(lhs, rhs);
                break;
            case GT_EQ:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpOGE(lhs, rhs);
                else
                    result = Builder.CreateICmpSGE(lhs, rhs);
                break;
            case EQUALS:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpOEQ(lhs, rhs);
                else
                    result = Builder.CreateICmpEQ(lhs, rhs);
                break;
            case NOTEQUAL:
                if (lhs->getType()->isFloatTy())
                    result = Builder.CreateFCmpONE(lhs, rhs);
                else
                    result = Builder.CreateICmpNE(lhs, rhs);
                break;
            default:
                // This shouldn't happen
                break;
        }

        return relation_pr(result, hintType);
    }
    else return lhs;
}

Value* Parser::term(Type* hintType)
{
    if (P_DEBUG) std::cout << "term" << '\n';

    Value* val = factor(hintType);
    return term_pr(val, hintType);
}

// Multiplication / Division 
Value* Parser::term_pr(Value* lhs, Type* hintType)
{
    if (P_DEBUG) std::cout << "term pr" << '\n';


    if (token() == TokenType::MULTIPLICATION 
        || token() == TokenType::DIVISION)
    {
        TokenType op = advance().type;
        Value* rhs = factor(hintType);

        // Type checking
        if (lhs->getType() == Type::getInt32Ty(TheContext)
            && rhs->getType() == Type::getFloatTy(TheContext))
        {
            // Convert lhs to float
            lhs = convert_type(lhs, Type::getFloatTy(TheContext));
        }
        else if (lhs->getType() == Type::getFloatTy(TheContext) 
            && rhs->getType() == Type::getInt32Ty(TheContext))
        {
            // Convert rhs to float
            rhs = convert_type(rhs, Type::getFloatTy(TheContext));
        }
        else if (lhs->getType() != rhs->getType() 
                    && lhs->getType() != Type::getFloatTy(TheContext)
                    && lhs->getType() != Type::getInt32Ty(TheContext))
        {
            // Types aren't the same or aren't both float/int
            err_handler->reportError("Term operations (multiplication and division) are only defined on float and integer types.", curr_token.line);
        }

        Value* result;

        if (op == TokenType::MULTIPLICATION)
        {
            if (lhs->getType()->isFloatTy())
                result = Builder.CreateFMul(lhs, rhs);
            else
                result = Builder.CreateMul(lhs, rhs);
        }
        else
        {
            if (lhs->getType()->isFloatTy())
                result = Builder.CreateFDiv(lhs, rhs);
            else
                result = Builder.CreateSDiv(lhs, rhs);
        }

        return term_pr(result, hintType);
    }
    else return lhs;
}

Value* Parser::factor(Type* hintType)
{
    if (P_DEBUG) std::cout << "factor" << '\n';
    Value* retval;

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
            MyValue mval = advance().val;
            APInt negated_int = APInt(32, mval.int_value);
            negated_int.negate();
            return ConstantInt::get(TheContext, negated_int);
        }
        else if (token() == TokenType::FLOAT) 
        {
            MyValue mval = advance().val;
            APFloat negated_float = APFloat(mval.float_value);
            negated_float.changeSign();
            return ConstantFP::get(TheContext, negated_float);
        }
        else if (token() == TokenType::IDENTIFIER) 
        {
            Value* nameVal = name(hintType);
            if (nameVal->getType() == Type::getInt32Ty(TheContext))
                return Builder.CreateNeg(nameVal);
            else if (nameVal->getType() == Type::getFloatTy(TheContext))
                return Builder.CreateFNeg(nameVal);
        }

        // One of the paths above should have returned
        // Being here is an error
        std::ostringstream stream;
        stream << "Bad token following negative sign: " << TokenTypeStrings[token()];
        err_handler->reportError(stream.str(), curr_token.line);
        advance();
    }
    else if (token() == TokenType::IDENTIFIER)
    {
        retval = name(hintType);
    }
    else if (token() == STRING)
    {
        MyValue mval = advance().val;
        //TODO
    }
    else if (token() == CHAR)
    {
        MyValue mval = advance().val;
        retval = ConstantInt::get(TheContext, APInt(8, mval.char_value));
    }
    else if (token() == INTEGER)
    {
        MyValue mval = advance().val;
        retval = ConstantInt::get(TheContext, APInt(32, mval.int_value));
    }
    else if (token() == FLOAT)
    {
        MyValue mval = advance().val;
        retval = ConstantFP::get(TheContext, APFloat(mval.float_value));
    }
    else if (token() == RS_TRUE || token() == RS_FALSE)
    {
        MyValue mval = advance().val;
        retval = ConstantInt::get(TheContext, APInt(1, mval.int_value));
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

Value* Parser::name(Type* hintType, bool load)
{
    if (P_DEBUG) std::cout << "name" << '\n';

    std::string id = require(TokenType::IDENTIFIER).val.string_value;

    // RS_IN - we expect to be able to read this variable's value
    SymTableEntry* entry = symtable_manager->resolve_symbol(id, true, RS_IN);

    Value* val;
    if (load)
        val = Builder.CreateLoad(entry->value, id);
    else
        val = entry->value;

    if (token() == TokenType::L_BRACKET)
    {
        // TODO deal with indexing 
        advance();
        expression(Type::getInt32Ty(TheContext));
        require(TokenType::R_BRACKET);
    }

    return val;
}

