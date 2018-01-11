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
    for (char k = '0'; k <= '9'; k++)
    {
        this->ascii_mapping[k] = CharClass::DIGIT;
    }
    for (char i = 'A'; i <= 'Z'; i++)
    {
        // Only uppercase letters are used unless it's within a string
        this->ascii_mapping[i] = CharClass::LETTER;
    }
    this->ascii_mapping['\t'] = CharClass::WHITESPACE;  
    this->ascii_mapping['\n'] = CharClass::WHITESPACE; 
    this->ascii_mapping[' '] = CharClass::WHITESPACE; 

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

void Scanner::consume(const char* until)
{
    char ch;
    bool checking = false;
    int idx = 0;
    while (this->input_file.get(ch))
    {
        if (!until[idx]) break;
        if (ch == until[idx])
        {
            checking = true;
            idx++;
        }
        else if (checking)
        {
            checking = false;
            idx = 0;
        }
    }
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
    while ((ch = this->input_file.peek()) 
            && getClass(ch) == CharClass::WHITESPACE)
    {
        if (ch == '\n') this->line_number++;

        this->input_file.get();
    }

    this->input_file.get(ch);
    // If the current token is a string or char, the letters should not 
    //  be uppercased. Otherwise, convert ch to upper for simpler scanning
    //if (token.type != TokenType::STRING && token.type != TokenType::CHAR)
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

        case '(':
            token.type = TokenType::L_PAREN;
            break;
        case ')':
            token.type = TokenType::R_PAREN;
            break;
         case '[':
            token.type = TokenType::L_BRACKET;
            break;
         case ']':
            token.type = TokenType::R_BRACKET;
            break;
         case ';':
            token.type = TokenType::SEMICOLON;
            break;
        case '"':
            // String token
            token.type = TokenType::STRING;
            // Need the extra scope level because k is defined in a case
            {
                int k = 0;
                while ((ch = this->input_file.get()) != '"')
                {
                    // TODO: check chars in the quote for being valid string chars 
                    token.val.string_value[k++] = ch;
                }
            }
            break;
        case '\'':
            token.type = TokenType::CHAR;
            this->input_file.get(ch);
            // TODO: Check ch for validity
            token.val.char_value = ch;
            break;
        case '*':
            if (this->input_file.peek() == '/')
            {
                block_comment_level--;
            }
            else 
            {
                token.type = TokenType::MULTIPLICATION;
                break;
            }
            break;
        case '+':
            token.type = TokenType::PLUS;
            break;
        case '-':
            token.type = TokenType::MINUS;
            break;
        case '=':
            if (this->input_file.peek() == '=')
            {
                this->input_file.get();
                token.type = TokenType::EQUAL;
            }
            break;
        case ':':
            if (this->input_file.peek() == '=') 
            {
                this->input_file.get();
                token.type = TokenType::ASSIGNMENT;
            }
            break;
        case '/':
            if (input_file.peek() == '/')
            {
                line_comment = true;
                // Consume all chars until \n
                consume("\n");
            }
            else if (input_file.peek() == '*')
            {
                block_comment_level++;
                // TODO: Consume all chars until */
                consume("*/");
            }
            else 
            {
                token.type = TokenType::DIVISION;
            }
            break;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0':
            token.type = TokenType::INTEGER;
            token.val.int_value = ch - '0';
            // TODO
            //while (this->input_file.get(ch))
            //{
            //    if (ch == '.')
            //    {
            //        token.type = TokenType::FLOAT;
            //    }
            //    else if (getClass(ch) != CharClass::DIGIT) 
            //    {
            //        break;
            //    }
            //    
            //    // TODO: What's a better way to do this? Is it easier to make
            //    //  a string first and call a to int method? Or what
            //    token.val.int_value += token.val.int_value * 10 + (ch - '0');
            //    // TODO: deal with floats
            //}
            break;
        default:
            // TODO: Turn this into a big case block for letters
            /*
            if (getClass(ch) == CharClass::LETTER)
            {
                for (int k = 0; 
                        getClass(ch) != CharClass::WHITESPACE; 
                        this->input_file.get(ch), k++)
                {
                    token.val.string_value[k] = ch; 
                }
            }
            */
            break;
    }

    // TODO: Check for file errors
    
    return token;
}

Scanner::~Scanner() { this->input_file.close(); }


