#pragma once
#include "token.h"
#include "errhandler.h"
#include "symboltable.h"

#include <fstream>
#include <sstream>
#include <ctype.h>
#include <string>

class Scanner
{
public:

    Scanner(ErrHandler* handler, SymbolTableManager* manager);

    /*
        Sets up the scanner to read from a file.
        Initializes variables in the class.

        filename - name of the file to read the program from

        returns - whether the scanner was successfully initialized 
                    (mainly whether the file was initialized for reading)
    */
    bool init(const char* filename);

    Token getToken(); // returns the next token in input_file

    ~Scanner();
private:
    ErrHandler* err_handler;
    SymbolTableManager* symtable_manager;

    std::ifstream input_file;
    int line_number;

    CharClass ascii_mapping[128] = {CharClass::SYMBOL};

    TokenType getWordTokenType(std::string str);

    bool isValidInIdentifier(char ch);
    bool isValidInString(char ch);
    bool isValidChar(char ch);

    void consumeWhitespaceAndComments();
};

