#include "scanner.h"
#include <iostream>
#include <ctype.h>

bool Scanner::init(const char* filename)
{
    this->input_file.open(filename, std::ifstream::in);

    if (this->input_file.bad()) return false;
    
    this->line_number = 1;

    // TODO: Init some tables (e.g. reserved words)


    // Init ascii character class mapping
    //this->ascii_mapping = {0};
    for (int k = 48; k < 58; k++)
    {
        this->ascii_mapping[k] = CharClass::DIGIT;
    }
    for (int i = 65; i < 91; i++)
    {
        // Only uppercase letters are used unless it's within a string
        this->ascii_mapping[i] = CharClass::LETTER;
    }
    //this->ascii_mapping[10] = CharClass::SYMBOLS
    //this->ascii_mapping[33] = CharClass::OPERATORS
    //this->ascii_mapping[34] = CharClass::
    //this->ascii_mapping[38] = CharClass::
    //this->ascii_mapping[39] = CharClass::
    //this->ascii_mapping[40] = CharClass::
    //this->ascii_mapping[41] = CharClass::
    //this->ascii_mapping[42] = CharClass::
    //this->ascii_mapping[43] = CharClass::
    //this->ascii_mapping[44] = CharClass::
    //this->ascii_mapping[45] = CharClass::
    //this->ascii_mapping[46] = CharClass::
    //this->ascii_mapping[47] = CharClass::
    //this->ascii_mapping[58] = CharClass::
    //this->ascii_mapping[59] = CharClass::
    //this->ascii_mapping[60] = CharClass::
    //this->ascii_mapping[61] = CharClass::
    //this->ascii_mapping[62] = CharClass::
    //this->ascii_mapping[91] = CharClass::
    //this->ascii_mapping[93] = CharClass::
    //this->ascii_mapping[95] = CharClass::

    //this->ascii_mapping[10] = ::NEWLINE;
    //this->ascii_mapping[33] = ::EXCLAIMATION_POINT;
    //this->ascii_mapping[34] = ::STR_QUOTE;
    //this->ascii_mapping[38] = ::AND;
    //this->ascii_mapping[39] = ::CHAR_QUOTE;
    //this->ascii_mapping[40] = ::L_PAREN;
    //this->ascii_mapping[41] = ::R_PAREN;
    //this->ascii_mapping[42] = ::MULTIPLY;
    //this->ascii_mapping[43] = ::PLUS;
    //this->ascii_mapping[44] = ::COMMA;
    //this->ascii_mapping[45] = ::MINUS;
    //this->ascii_mapping[46] = ::PERIOD;
    //this->ascii_mapping[47] = ::FWD_SLASH;
    //this->ascii_mapping[58] = ::COLON;
    //this->ascii_mapping[59] = ::SEMICOLON;
    //this->ascii_mapping[60] = ::LT;
    //this->ascii_mapping[61] = ::EQUALS;
    //this->ascii_mapping[62] = ::GT;
    //this->ascii_mapping[91] = ::L_BRACKET;
    //this->ascii_mapping[93] = ::R_BRACKET;
    //this->ascii_mapping[95] = ::UNDERSCORE;
    //this->ascii_mapping[124] = ::OR;
    //for (int k = 0; k < 128; k++) 
    //{
    //    std::cout << k << ' ' << ascii_mapping[k] << '\n';
    //}

    return true;
}

CharClass Scanner::getClass(char c)
{
    CharClass type = this->ascii_mapping[c];
    return type;
}

Token Scanner::getToken()
{
    // The token to be returned; defaults to an identifier with no val
    Token token;
    token.type = TokenType::IDENTIFIER;

    if (this->input_file.eof())
    {
        token.type = TokenType::FILE_END;
        return token;
    }

    // Comment handling
    // Block comment level is saved to support nested block comments
    // A level of 0 indicates the current token is not a block comment 
    int block_comment_level = 0;
    bool line_comment = false;

    // Stores next char read from file
    char ch;

    // Consume all leading whitespace 
    while ((ch = this->input_file.peek()) && (ch == ' ' || ch == '\t' || ch == '\n'))
    {
        // Throw away whitespace char
        this->input_file.get();
    }

    // Iterate through to get the next token (May not need the while?)
    //while (
        this->input_file.get(ch)
        ;
    //)
    {
        // If the current token is a string or char, the letters should not 
        //  be uppercased. Otherwise, convert ch to upper for simpler scanning
        if (token.type != TokenType::STRING && token.type != TokenType::CHAR)
        {
            ch = toupper(ch);
        }
        
        switch (ch)
        {
/*
            case '\n':
                // Newlines terminate line comments and increment 
                //  the line_number counter
                this->line_number++;
                line_comment = false;
                // No break - move on to whitespace parsing
            case ' ':
            case '\t': 
                // TODO: Whitespace (break)
                break;
*/
            case '*':
                if (input_file.peek() == '/')
                {
                    block_comment_level--;
                }
            case '/':
                if (input_file.peek() == '/')
                {
                    line_comment = true;
                }
                else if (input_file.peek() == '*')
                {
                    block_comment_level++;
                }
                else 
                {
                    token.type = TokenType::DIVISION;
                }
                break;
        }
    }
    // TODO: Check for file errors

    
    return token;
}

Scanner::~Scanner() { this->input_file.close(); }


