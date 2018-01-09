#include "scanner.h"

bool Scanner::init(char* filename)
{
    this->input_file.open(filename, std::ifstream::in);
    this->line_number = 1;
    // TODO: Init some tables (e.g. reserved words)


}

Token Scanner::getToken()
{
    // Comment handling
    // Block comment level is saved to support nested block comments
    // A level of 0 indicates the current token is not a block comment 
    int block_comment_level = 0;
    bool line_comment = false;

    // Stores next char read from file
    char ch;

    // The token to be returned; defaults to an identifier with no val
    Token token;
    token.type = TokenType::IDENTIFIER;

    while (this->input_file.get(ch))
    {
        // ch contains next char
        // TODO: break once a complete token is found
        switch (ch)
        {
            case '\t': break;
            case ' ':
                if (token.type == TokenType::STRING || token.type == TokenType::CHAR)
                {
                    // Spaces are allowed inside strings/chars only, 
                    //  otherwise they're ignored
                    // TODO
                }
                break;
            case '/':
                if (input_file.peek() == '/')
                {
                    line_comment = true;
                }
                break;
            case '\n':
                this->line_number++;
                line_comment = false;
                break;
        }
    }
    // TODO: Check for EOF or file errors
    // Maybe set a flag in Scanner to indicate EOF so all subsequent calls 
    //  to getToken return an EOF type token

    return token;
}

Scanner::~Scanner() { this->input_file.close(); }


