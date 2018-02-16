#include "parser.h"

const char* TokenTypeStrings[] = 
{
".", ";", "(", ")", ",", "[", "]", ":", "&", "|", "+", "-", "<", ">", "<=", ">=", ":=", "==", "!=", "*", "/", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "IDENTIFIER", "UNKNOWN",
"RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT"
};

Parser::Parser(Scanner* scan, ErrHandler* handler) 
    : scanner(scan), err_handler(handler) { }

TokenType Parser::token()
{
    if (!curr_token_valid)
    {
        curr_token_valid = true;
        curr_token = scanner->getToken();
        std::cout << "\tGot: " << TokenTypeStrings[curr_token.type] << '\n';
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
Token Parser::require(TokenType t)
{
    TokenType type = token();
    advance();
    if (t != type) 
    {
        // Report err
        std::ostringstream stream;
        stream << "Bad Token: " << TokenTypeStrings[type] << "\tExpected: " << TokenTypeStrings[t];
        err_handler->reportError(stream.str(), curr_token.line);
    }
    return curr_token;
}


void Parser::parse() 
{
    program();
    require(TokenType::FILE_END);
}

void Parser::program()
{
    std::cout << "program" << '\n';
    program_header(); 
    program_body(); 
    require(TokenType::PERIOD);
}

void Parser::program_header()
{
    std::cout << "program header" << '\n';
    require(TokenType::RS_PROGRAM);

    require(TokenType::IDENTIFIER);
    std::string program_name = curr_token.val.string_value;
    std::cout << "Prog. name is " << program_name << '\n';

    require(TokenType::RS_IS);
}

void Parser::program_body()
{
    std::cout << "program body" << '\n';
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
    std::cout << "declaration" << '\n';
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
    std::cout << "proc decl" << '\n';
    proc_header();
    proc_body();
}

void Parser::proc_header()
{
    std::cout << "proc header" << '\n';
    require(TokenType::RS_PROCEDURE);
    require(TokenType::IDENTIFIER);
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        parameter_list(); 
    require(TokenType::R_PAREN);
}

void Parser::proc_body()
{
    std::cout << "proc body" << '\n';
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
    std::cout << "param list" << '\n';
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
    std::cout << "param" << '\n';

    var_declaration(false);

    TokenType param_type = token(); // IN|OUT|INOUT
    std::cout << "Param type: " << TokenTypeStrings[param_type] << '\n';
    advance();
}


void Parser::var_declaration(bool is_global)
{
    std::cout << "var decl" << '\n';
    type_mark();
    std::string id = require(TokenType::IDENTIFIER).val.string_value;
    std::cout << "Identifier: " << id << '\n';

    if (token() == TokenType::L_BRACKET)
    {
        advance();

        lower_bound();
        require(TokenType::COLON);
        upper_bound();
        require(TokenType::R_BRACKET);
    }
    // TODO: Check if this idenfitier is declared already, if so report err
}

void Parser::type_mark()
{
    std::cout << "type_mark" << '\n';

    TokenType typemark = token();
    std::cout << TokenTypeStrings[typemark] << '\n';

    advance();
}

void Parser::lower_bound()
{
    std::cout << "lower_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    require(TokenType::INTEGER);
}

void Parser::upper_bound()
{
    std::cout << "upper_bound" << '\n';
    // Minus allowed in spec now
    if (token() == TokenType::MINUS) advance();
    require(TokenType::INTEGER);
}

bool Parser::statement()
{
    std::cout << "stmnt" << '\n';

    // TODO: Make return value more throrough; maybe check the inner stmnts
    
    if (token() == TokenType::IDENTIFIER)
        identifier_statement();
    else if (token() == TokenType::RS_IF)
        if_statement();
    else if (token() == TokenType::RS_FOR)
        loop_statement();
    else if (token() == TokenType::RS_RETURN)
        return_statement();
    else return false; // TODO: ERR

    return true;
}

// Groups assignment and proc call statements, as both
//  start with an identifier
void Parser::identifier_statement()
{
    std::cout << "identifier stmnt" << '\n';
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
    std::cout << "assignment stmnt" << '\n';
    // already have identifier; need to check for indexing first
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression();
        require(TokenType::R_BRACKET);
    }
    require(TokenType::ASSIGNMENT);
    expression();
}

void Parser::proc_call(std::string identifier)
{
    std::cout << "proc call" << '\n';
    // already have identifier
    require(TokenType::L_PAREN);
    if (token() != TokenType::R_PAREN)
        argument_list();
    require(TokenType::R_PAREN);
}

void Parser::if_statement()
{
    std::cout << "if" << '\n';
    require(TokenType::RS_IF);
    require(TokenType::L_PAREN);
    // TODO expression should return something
    expression();
    require(TokenType::R_PAREN);
    require(TokenType::RS_THEN);

    // TODO: Check condition
    //TODO handle else
    bool first_stmnt = true;
    while (true)
    {
        // Make sure there is at least one valid statement
        bool valid = statement();
        if (!valid && first_stmnt)
        {
            err_handler->reportError("No statement in IF body");
        }
        first_stmnt = false;
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
        // TODO control execution properly
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
    std::cout << "loop" << '\n';
    require(TokenType::RS_FOR);
    require(TokenType::L_PAREN);
    require(TokenType::IDENTIFIER);
    assignment_statement(curr_token.val.string_value); 
    require(TokenType::SEMICOLON);
    expression();
    require(TokenType::R_PAREN);
    // TODO: Check conditions
    while (true)
    {
        statement();
        require(TokenType::SEMICOLON);
        if (token() == TokenType::RS_END) break;
    }
    require(TokenType::RS_END); // Just to be sure 
    require(TokenType::RS_FOR);
}

void Parser::return_statement()
{
    std::cout << "return" << '\n';
    require(TokenType::RS_RETURN);
}

void Parser::argument_list()
{
    std::cout << "arg list" << '\n';
    while (true)
    {
        expression();
        if (token() == TokenType::COMMA) 
        {
            advance();
            continue;
        }
        else return;
    }
}

void Parser::destination()
{
    std::cout << "destination" << '\n';
    require(TokenType::IDENTIFIER);
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression();
        require(TokenType::R_BRACKET);
    }
}

void Parser::expression()
{
    std::cout << "expr" << '\n';

    arith_op(); 
    expression_pr();
}

void Parser::expression_pr()
{
    std::cout << "expr prime" << '\n';

    // TODO: Might need to separate & and | code for code generation stage
    if (token() == TokenType::AND || token() == TokenType::OR)
    {
        advance();
        arith_op();
        expression_pr();
    }
}

void Parser::arith_op()
{
    std::cout << "arith op" << '\n';
    if (token() == TokenType::RS_NOT)
    {
        advance();
    }

    relation();
    arith_op_pr();
}

void Parser::arith_op_pr()
{
    std::cout << "arith op pr" << '\n';
    if (token() == TokenType::PLUS || token() == TokenType::MINUS)
    {
        advance();
        relation(); 
        arith_op_pr();
    }
}

void Parser::relation()
{
    std::cout << "relation" << '\n';

    term();
    relation_pr();
}

void Parser::relation_pr()
{
    std::cout << "relation pr" << '\n';
    if ((token() == TokenType::LT)
        | (token() == TokenType::GT)
        | (token() == TokenType::LT_EQ)
        | (token() == TokenType::GT_EQ)
        | (token() == TokenType::EQUALS)
        | (token() == TokenType::NOTEQUAL))
    {
        advance();
        term();
        relation_pr();
    }
    else return;
}

void Parser::term()
{
    std::cout << "term" << '\n';

    factor();
    term_pr();
}

Value Parser::term_pr()
{
    std::cout << "term pr" << '\n';

    // TODO: assign to this
    Value value;

    if (token() == TokenType::MULTIPLICATION)
    {
        advance();
        factor();
        term_pr();
    }
    else if (token() == TokenType::DIVISION)
    {
        advance();
        factor();
        term_pr();
    }
    return value;
}

Value Parser::factor()
{
    std::cout << "factor" << '\n';
    Value retval;

    // Token is either (expression), name, number, string, char, true, false
    if (token() == TokenType::L_PAREN)
    {
        advance();
        // TODO: expression should return a value
        //retval = expression();
        expression();
        require(TokenType::R_PAREN);
    }
    else if (token() == TokenType::MINUS)
    {
        advance();
        // TODO do seomething with the minus
    }

    if (token() == TokenType::STRING 
            || token() == TokenType::CHAR 
            || token() == TokenType::RS_TRUE 
            || token() == TokenType::RS_FALSE 
            || token() == TokenType::INTEGER
            || token() == TokenType::FLOAT)
    {
        // Literal values
        // Consume the token and get the value
        retval = advance().val;
        // TODO: type checking how do
    }
    else if (token() == TokenType::IDENTIFIER)
    {
        retval = name();
    }
    return retval;
}

Value Parser::name()
{
    std::cout << "name" << '\n';
    require(TokenType::IDENTIFIER);
    if (token() == TokenType::L_BRACKET)
    {
        advance();
        expression();
        require(TokenType::R_BRACKET);
    }
    Value val;
    return val;
}

