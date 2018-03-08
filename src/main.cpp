#include "token.h"
#include "scanner.h"
#include "parser.h"

#include <vector>
#include <iostream>


void compile(char* filename, ErrHandler* err_handler)
{
    std::cout << "\nCompiling " << filename << '\n';
    SymbolTableManager* sym_manager = new SymbolTableManager(err_handler);

    Scanner* scanner = new Scanner(err_handler, sym_manager);
    if (!scanner->init(filename))
    {
        err_handler->reportError("Scanner initialization failed. Ensure the input file is valid.");
        return;
    }

    // Parse the tokens
    Parser* parser = new Parser(err_handler, sym_manager, scanner);
    parser->parse();

    // Delete instances
    delete sym_manager;
    delete scanner;
    delete parser;
}

/*
Return codes
1 - No filename given
2 - Empty file or I/O error (no tokens given by scanner)
3 - Some errors reported by err_handler
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
        std::cout << err_handler->errors << " error(s) reported during scanner/parser phase.\n";
        return 3;
    }   

    return 0;
}

