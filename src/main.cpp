#include "scanner.h"
#include <vector>

// DEBUG

#include <iostream>
const char* TokenTypeStrings[] = 
{
    "PERIOD", "SEMICOLON", "L_PAREN", "R_PAREN", "COMMA", "L_BRACKET", "R_BRACKET", "COLON", "AND", "OR", "PLUS", "MINUS", "LT", "GT", "LT_EQ", "GT_EQ", "ASSIGNMENT", "EQUALS", "NOTEQUAL", "MULTIPLICATION", "DIVISION", "FILE_END", "STRING", "CHAR", "INTEGER", "FLOAT", "BOOL", "RS_IN", "RS_OUT", "RS_INOUT", "RS_PROGRAM", "RS_IS", "RS_BEGIN", "RS_END", "RS_GLOBAL", "RS_PROCEDURE", "RS_STRING", "RS_CHAR", "RS_INTEGER", "RS_FLOAT", "RS_BOOL", "RS_IF", "RS_THEN", "RS_ELSE", "RS_FOR", "RS_RETURN", "RS_TRUE", "RS_FALSE", "RS_NOT", "IDENTIFIER", "UNKNOWN"
};

// Gets a vector of tokens from the scanner
void scanner_debug(Scanner* scanner) 
{
    Token token;
    while ((token = scanner->getToken()).type != TokenType::FILE_END)
    {
        std::cout << token.line << '\t';
        std::cout << TokenTypeStrings[token.type] << '\t';
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::STRING)
        {
            std::cout << '"' << token.val.string_value << '"';
        }
        if (token.type == TokenType::CHAR)
        {
            std::cout << '\'' << token.val.char_value << '\'';
        }
        if (token.type == TokenType::INTEGER)
        {
            std::cout << token.val.int_value;
        }
        if (token.type == TokenType::FLOAT)
        {
            std::cout << token.val.double_value;
        }
        std::cout << std::endl;
    }
}

// END DEBUG

/*
Return codes
1 - No filename given
2 - Empty file or I/O error (no tokens given by scanner)
*/
int main(int argc, char** argv)
{

    ErrHandler* err_handler = new ErrHandler();

    if (argc < 2) 
    {
        err_handler->reportError("No filename provided.");
        return 1;
    }

    Scanner* scanner = new Scanner();
    bool init_success = scanner->init(argv[1]);
    if (!init_success)
    {
        err_handler->reportError("Scanner initialization failed. Ensure the input file is valid.");
        return 2;
    }

    scanner->setErrHandler(err_handler);

    // TODO: Scanner debugging only. 
    // This should be commented out when parsing (scanner_debug consumes tokens)
    scanner_debug(scanner);

    if (err_handler->has_errors)
    {
        std::cout << "Some errors reported during scanner debug phase.";
        return 3;
    }




    //TODO Parse the tokens

    // The parser takes the scanner as input and gets tokens one at a time
    //Parser parser(scanner);
    //parser.parse();

    //TODO Type checking and other tasks
    
    return 0;
}

