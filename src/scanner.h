#pragma once
#include "token.h"
#include "errhandler.h"

#include <fstream>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <string>

enum CharClass 
{
    SYMBOL=0, LETTER, DIGIT, WHITESPACE
};

class Scanner
{
public:

    Scanner(ErrHandler* handler);

    /*
        Sets up the scanner to read from a file.
        Initializes variables in the class.

        filename - name of the file to read the program from

        returns - whether the scanner was successfully initialized 
                    (mainly whether the file was initialized for reading)
    */
    bool init(const char* filename);

    /*
        returns - The next token in input_file
    */
    Token getToken();
    ~Scanner();
private:
    std::ifstream input_file;
    int line_number;

    ErrHandler* err_handler;

    CharClass ascii_mapping[128] = {CharClass::SYMBOL};
    std::unordered_map<std::string, TokenType> reserved_words_map;

    TokenType getWordTokenType(std::string str);

    bool isValidInIdentifier(char ch);
    bool isValidInString(char ch);
    bool isValidChar(char ch);
    bool isValidShared(char ch);

    void consumeWhitespaceAndComments();
};

