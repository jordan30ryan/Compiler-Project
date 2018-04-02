#include "token.h"
#include "errhandler.h"
#include "symboltable.h"
#include "scanner.h"
#include "parser.h"

#include "llvm/IR/Module.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>


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
    std::unique_ptr<llvm::Module> TheModule = parser->parse();

    // Compile the IR to a file
    filenamestr.append(".ll");
    compile_to_file(std::move(TheModule), filenamestr);

    // Delete instances
    delete sym_manager;
    delete scanner;
    delete parser;

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

    if (err_handler->warnings)
    {
        std::cerr << err_handler->warnings << " warning(s) reported\n";
    }
    if (err_handler->errors)
    {
        std::cerr << err_handler->errors << " error(s) reported\n";
        return 2;
    }   

    return 0;
}

