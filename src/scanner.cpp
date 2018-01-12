#include "scanner.h"
#include <iostream>
#include <ctype.h>
#include <string.h>

bool Scanner::init(const char* filename)
{
    this->input_file.open(filename, std::ifstream::in);

    if (this->input_file.bad()) return false;
    
    this->line_number = 1;

    // Init reserved words table
    reserved_words_map.emplace_back(TokenType::RS_IN, "IN");
    reserved_words_map.emplace_back(TokenType::RS_OUT, "OUT");
    reserved_words_map.emplace_back(TokenType::RS_INOUT, "INOUT");
    reserved_words_map.emplace_back(TokenType::RS_PROGRAM, "PROGRAM");
    reserved_words_map.emplace_back(TokenType::RS_IS, "IS");
    reserved_words_map.emplace_back(TokenType::RS_BEGIN, "BEGIN");
    reserved_words_map.emplace_back(TokenType::RS_END, "END");
    reserved_words_map.emplace_back(TokenType::RS_GLOBAL, "GLOBAL");
    reserved_words_map.emplace_back(TokenType::RS_PROCEDURE, "PROCEDURE");
    reserved_words_map.emplace_back(TokenType::RS_CHAR, "CHAR");
    reserved_words_map.emplace_back(TokenType::RS_INTEGER, "INTEGER");
    reserved_words_map.emplace_back(TokenType::RS_FLOAT, "FLOAT");
    reserved_words_map.emplace_back(TokenType::RS_BOOL, "BOOL");
    reserved_words_map.emplace_back(TokenType::RS_IF, "IF");
    reserved_words_map.emplace_back(TokenType::RS_THEN, "THEN");
    reserved_words_map.emplace_back(TokenType::RS_ELSE, "ELSE");
    reserved_words_map.emplace_back(TokenType::RS_FOR, "FOR");
    reserved_words_map.emplace_back(TokenType::RS_RETURN, "RETURN");
    reserved_words_map.emplace_back(TokenType::RS_TRUE, "TRUE");
    reserved_words_map.emplace_back(TokenType::RS_FALSE, "FALSE");
    reserved_words_map.emplace_back(TokenType::RS_NOT, "NOT");

    // Init ascii character class mapping
    for (char k = '0'; k <= '9'; k++)
    {
        this->ascii_mapping[k] = CharClass::DIGIT;
    }
    for (char i = 'A', j = 'a'; i <= 'Z'; i++, j++)
    {
        this->ascii_mapping[i] = CharClass::LETTER;
        this->ascii_mapping[j] = CharClass::LETTER;
    }

    this->ascii_mapping['\t'] = CharClass::WHITESPACE;  
    this->ascii_mapping['\n'] = CharClass::WHITESPACE; 
    this->ascii_mapping['\r'] = CharClass::WHITESPACE; 
    this->ascii_mapping[' '] = CharClass::WHITESPACE; 

    this->ascii_mapping['_'] = CharClass::UNDERSCORE;

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

bool Scanner::isValidIdentifier(char ch)
{
    CharClass cls = getClass(ch);
    return cls == CharClass::LETTER 
            || cls == CharClass::DIGIT 
            || cls == CharClass::UNDERSCORE;

}

TokenType Scanner::getWordTokenType(char* str)
{
    // TODO: Optimize. Use hashing?
    for (int k = 0; k < this->reserved_words_map.size(); k++)
    {
        if (!strcmp(this->reserved_words_map[k].value, str))
        {
            return this->reserved_words_map[k].type;
        }
    }
    return TokenType::IDENTIFIER;
}

Token Scanner::getToken()
{
    // The token to be returned; defaults to an identifier with no val
    Token token;
    token.type = TokenType::UNKNOWN;

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

    if (this->input_file.eof())
    {
        token.type = TokenType::FILE_END;
        return token;
    }
        
    switch (ch)
    {
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
        case '.':
            token.type = TokenType::PERIOD;
            break;
        case ',':
            token.type = TokenType::COMMA;
            break;
        case '&':
            token.type = TokenType::AND;
            break;
        case '|':
            token.type = TokenType::OR;
            break;
        case '<':
            if (this->input_file.peek() == '=')
            {
                this->input_file.get();
                token.type = TokenType::LT_EQ;
            }
            else 
                token.type = TokenType::LT;

            break;
        case '>':
            if (this->input_file.peek() == '=')
            {
                this->input_file.get();
                token.type = TokenType::GT_EQ;
            }
            else 
                token.type = TokenType::GT;

            break;
        case '"':
            // String token
            token.type = TokenType::STRING;
            // Need the extra scope level because k is defined in a case
            {
                int k = 0;
                while (ch = this->input_file.get() && ch != '"')
                {
                    // TODO: check chars in the quote for being valid string chars 
                    token.val.string_value[k++] = ch;
                }
                if (ch != '"') 
                {
                    // TODO: error: reached EOF and string quotes were never closed.
                }
                token.val.string_value[k] = 0;
            }
            break;
        case '\'':
            token.type = TokenType::CHAR;
            this->input_file.get(ch);
            // TODO: Check ch for validity
            token.val.char_value = ch;
            this->input_file.peek(ch);
            if (ch != '\'')
            {
                // TODO: error: single quote containing more than one char
            }
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
                token.type = TokenType::EQUALS;
            }
            break;
        case ':':
            if (this->input_file.peek() == '=') 
            {
                this->input_file.get();
                token.type = TokenType::ASSIGNMENT;
            }
            else 
            {
                token.type = TokenType::COLON;
            }
            break;

        case '!':
            if (this->input_file.peek() == '=') 
            {
                this->input_file.get();
                token.type = TokenType::NOTEQUAL;
            }
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
            token.val.int_value = (int)(ch - '0');
            while (this->input_file.get(ch))
            {
                if (ch == '.')
                {
                    token.type = TokenType::FLOAT;
                }
                else if (getClass(ch) != CharClass::DIGIT) 
                {
                    this->input_file.putback(ch);
                    break;
                }
                
                // TODO: What's a better way to do this? Is it easier to make
                //  a string first and call a to int method? Or what
                token.val.int_value = 10 * token.val.int_value + (int)(ch - '0');
                // TODO: deal with floats
            }
            break;
        default:
            // TODO: Turn this into a big case block for letters?
            if (getClass(ch) == CharClass::LETTER)
            {
                int k;
                for (k = 0; k < MAX_STRING_LEN, isValidIdentifier(ch); k++)
                {
                    token.val.string_value[k] = toupper(ch);
                    this->input_file.get(ch);
                }
                // Put back most recently read char
                this->input_file.putback(ch);
                // Null terminate the string
                token.val.string_value[k] = 0;

                // Check whether this is a reserved word or identifier
                token.type = getWordTokenType(token.val.string_value);
            }
            break;
    }

    // TODO: Check for file errors

    // For debugging purposes mostly
    if (token.type == TokenType::UNKNOWN)
    {
        token.val.char_value = ch;
    }

    return token;
}

Scanner::~Scanner() { this->input_file.close(); }


