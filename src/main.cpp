#include "token.h"
#include "scanner.h"
#include "parser.h"

#include <vector>
#include <iostream>


bool compile(char* filename, ErrHandler* err_handler)
{
    // Remove extension from input filename
    std::string filenamestr(filename);
    int extidx = filenamestr.find(".src");
    if (extidx > 0)
        filenamestr = filenamestr.substr(0, extidx);

    std::cout << "Compiling: " << filenamestr << '\n';

    SymbolTableManager* sym_manager = new SymbolTableManager(err_handler);

    Scanner* scanner = new Scanner(err_handler, sym_manager);
    if (!scanner->init(filename))
    {
        err_handler->reportError("Scanner initialization failed. Ensure the input file is valid.");
        return false;
    }


    // Parse the tokens
    Parser* parser = new Parser(err_handler, sym_manager, scanner, filenamestr);
    parser->parse();

    // Delete instances
    delete sym_manager;
    delete scanner;
    delete parser;

    // TODO: Compile ll / link with runtime?
    return true;
}

/*
Return codes
1 - No filename given
2 - Some errors reported by err_handler
*/
int main(int argc, char** argv)
{
    ErrHandler* err_handler = new ErrHandler();

    if (argc < 2) 
    {
        err_handler->reportError("No filename provided.");
        return 1;
    }

    for (int k = 1; k < argc; k++)
    {
        compile(argv[k], err_handler);
    }

    if (err_handler->errors)
    {
        std::cerr << err_handler->errors << " error(s) reported during scanner/parser phase.\n";
        return 2;
    }   

    return 0;
}

