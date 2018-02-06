#include "scanner.h"
#include <iostream>
#include <ctype.h>
#include <string.h>

Scanner::Scanner(ErrHandler* handler) : err_handler(handler) {}

bool Scanner::init(const char* filename)
{
    input_file.open(filename, std::ifstream::in);
    if (!input_file.is_open() || input_file.bad()) return false;
    
    line_number = 1;

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
    reserved_words_map.emplace_back(TokenType::RS_STRING, "STRING");
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
        ascii_mapping[k] = CharClass::DIGIT;
    }
    for (char i = 'A', j = 'a'; i <= 'Z'; i++, j++)
    {
        ascii_mapping[i] = CharClass::LETTER;
        ascii_mapping[j] = CharClass::LETTER;
    }

    ascii_mapping['\t'] = CharClass::WHITESPACE;  
    ascii_mapping['\n'] = CharClass::WHITESPACE; 
    ascii_mapping['\r'] = CharClass::WHITESPACE; 
    ascii_mapping[' '] = CharClass::WHITESPACE; 

    return true;
}

// Check if ch is a valid identifier character
bool Scanner::isValidInIdentifier(char ch)
{
    CharClass cls = ascii_mapping[ch];
    return cls == CharClass::LETTER 
            || cls == CharClass::DIGIT 
            || ch == '_';
}

bool Scanner::isValidInString(char ch)
{
    return isValidShared(ch) || ch == ',' || ch == '\'';
}

bool Scanner::isValidChar(char ch)
{
    return isValidShared(ch) || ch == '"';
}

// Simplifies checking for valid string or char literal characters by 
//  checking for the chars shared by each type
bool Scanner::isValidShared(char ch)
{
    return isValidInIdentifier(ch) || ch == ' ' || ch == ';' || ch == ':' || ch == '.';
}

// Return the proper TokenType if str is a reserved word.
// Otherwise, str is interpreted as an identifier.
TokenType Scanner::getWordTokenType(char* str)
{
    // TODO: Optimize. Use hashing?
    for (unsigned int k = 0; k < reserved_words_map.size(); k++)
    {
        if (!strcmp(reserved_words_map[k].value, str))
        {
            return reserved_words_map[k].type;
        }
    }
    return TokenType::IDENTIFIER;
}

// Consume all leading whitespace and comments first
void Scanner::consumeWhitespaceAndComments()
{
    // Buffer
    char ch;
    while ((ch = input_file.peek()) 
            && ((ascii_mapping[ch] == CharClass::WHITESPACE) || ch == '/'))
    {
        if (ch == '/')
        {
            // First, consume the / so we can peek the next char 
            input_file.get();
            ch = input_file.peek();
            if (ch == '/')
            {
                // Consume line comment
                while (input_file.get() != '\n') {}
                line_number++;
            }
            else if (ch == '*')
            {
                input_file.get(); // Consume the *

                // Support nested comments
                int comment_level = 1;
                
                // Consume block comment
                while (input_file.get(ch))
                {
                    if (ch == '*' && input_file.peek() == '/')
                    {
                        input_file.get();
                        comment_level--;
                        if (comment_level == 0) break;
                    }
                    else if (ch == '/' && input_file.peek() == '*')
                    {
                        input_file.get();
                        comment_level++;
                    }
                    else if (ch == '\n') line_number++;
                }
            }
            else 
            {
                // The / was not followed by a / or *, so it's not a comment.
                // Put it back and let the switch handle it normally
                input_file.unget();
                break;
            }
        }
        else
        {
            // Consume the whitespace token
            input_file.get();

            if (ch == '\n') line_number++;
        }
    }
}

Token Scanner::getToken()
{
    // The token to be returned; defaults to unknown
    Token token;
    token.type = TokenType::UNKNOWN;

    // Stores next char read from file
    char ch;

    consumeWhitespaceAndComments();

    token.line = line_number;

    // Store next char of file in ch
    input_file.get(ch);

    // Check for EOF
    if (input_file.eof())
    {
        token.type = TokenType::FILE_END;
        return token;
    }

    // Main switch to get token type (and value if necessary)
    switch (ascii_mapping[ch])
    {
    case CharClass::WHITESPACE:
        // Something in consumeWhitespace... is not working correctly...
        break;
    case CharClass::LETTER:
        // Identifiers/Reserved words must all start with a letter
        int k;
        for (k = 0; k < MAX_STRING_LEN && isValidInIdentifier(ch); k++)
        {
            token.val.string_value[k] = toupper(ch);
            input_file.get(ch);
        }
        // Put back most recently read char (it wasn't valid in an identifier)
        input_file.unget();
        // Null terminate the string
        token.val.string_value[k] = 0;

        // Check whether this is a reserved word or identifier
        token.type = getWordTokenType(token.val.string_value);
        break;
    case CharClass::DIGIT:
        // TODO: Do I need to differentiate between int/float?
        // Extra scope level needed becuase of variables defined in this case
        {
            //token.type = TokenType::INTEGER;
            token.type = TokenType::NUMBER;

            token.val.int_value = (int)(ch - '0');

            bool is_fractional_part = false;
            double fract_mult = 0.1;

            while (input_file.get(ch))
            {
                if (ch == '.')
                {
                    //token.type = TokenType::FLOAT;
                    token.val.double_value = token.val.int_value;
                    is_fractional_part = true;
                    continue;
                }
                else if (ascii_mapping[ch] != CharClass::DIGIT) 
                {
                    if (ch == '_') continue;
                    input_file.unget();
                    break;
                }
                
                // Is there a better way to do this?
                if (is_fractional_part)
                {
                    token.val.double_value += fract_mult * (ch - '0');
                    fract_mult *= 0.1;
                }
                else 
                {
                    token.val.int_value = 10 * token.val.int_value + (int)(ch - '0');
                }
            }
            // TODO: Probably don't need to differentiate; but just to be sure
            token.val.double_value = token.val.int_value;
        }
        break;
    case CharClass::SYMBOL:
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
        case '+':
            token.type = TokenType::PLUS;
            break;
        case '-':
            token.type = TokenType::MINUS;
            break;
        case '/':
            // Comments get filtered out above
            token.type = TokenType::DIVISION;
            break;
        case '*':
            token.type = TokenType::MULTIPLICATION;
            break;
        case '&':
            token.type = TokenType::AND;
            break;
        case '|':
            token.type = TokenType::OR;
            break;
        case '<':
            if (input_file.peek() == '=')
            {
                input_file.get();
                token.type = TokenType::LT_EQ;
            }
            else 
                token.type = TokenType::LT;

            break;
        case '>':
            if (input_file.peek() == '=')
            {
                input_file.get();
                token.type = TokenType::GT_EQ;
            }
            else 
                token.type = TokenType::GT;

            break;
        case '"':
            // Need the extra scope level because k is defined in the case
            {
                // String token
                token.type = TokenType::STRING;

                int k = 0;
                while (input_file.get(ch) && ch != '"')
                {
                    if (!isValidInString(ch))
                    {
                        std::string err = "Char not valid in a string: ";
                        err += ch;
                        err_handler->reportError(err, line_number);
                    }
                    else 
                    {
                        token.val.string_value[k++] = ch;
                    }
                }
                if (ch != '"') 
                {
                    err_handler->reportError("Reached EOF and string quotes were never closed.", line_number);
                }
                token.val.string_value[k] = 0;
            }
            break;
        case '\'':
            token.type = TokenType::CHAR;
            input_file.get(ch);
            if (!isValidChar(ch))
            {
                std::string err = "Not a valid char literal: ";
                err += ch;
                err_handler->reportError(err, line_number);
            }
            token.val.char_value = ch;
            ch = input_file.get();
            if (ch != '\'')
            {
                err_handler->reportError("Single quote containing more than one char", line_number);
            }
            break;
        case '=':
            if (input_file.peek() == '=')
            {
                input_file.get();
                token.type = TokenType::EQUALS;
            }
            break;
        case ':':
            if (input_file.peek() == '=') 
            {
                input_file.get();
                token.type = TokenType::ASSIGNMENT;
            }
            else 
            {
                token.type = TokenType::COLON;
            }
            break;
        case '!':
            if (input_file.peek() == '=') 
            {
                input_file.get();
                token.type = TokenType::NOTEQUAL;
            }
            break;
        }
    }
    
    if (token.type == TokenType::UNKNOWN)
    {
        std::string err = "Unknown token: ";
        err += ch;
        err_handler->reportError(err, line_number);
    }

    return token;
}

Scanner::~Scanner() { input_file.close(); }

