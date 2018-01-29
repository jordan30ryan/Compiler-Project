#include "scanner.h"
#include "parser.h"
#include <vector>

#include <iostream>
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

    // Init scanner
    Scanner* scanner = new Scanner(err_handler);
    if (!scanner->init(argv[1]))
    {
        err_handler->reportError("Scanner initialization failed. Ensure the input file is valid.");
        return 2;
    }

    // Parse the tokens

    // The parser is constructed with a reference to the scanner
    //  so it can consume tokens one at a time.
    Parser parser(scanner);
    parser.parse();

// DEBUG
    if (err_handler->errors)
    {
        std::cout << err_handler->errors << " error(s) reported during scanner debug phase.\n";
    }    
// END DEBUG

    //TODO Type checking and other tasks
    
    return 0;
}

