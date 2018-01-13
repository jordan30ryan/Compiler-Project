#include "scanner.h"
#include <iostream>
#include <ctype.h>
#include <string.h>

bool Scanner::init(const char* filename)
{
    input_file.open(filename, std::ifstream::in);
    if (input_file.bad()) return false;
    
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

    ascii_mapping['_'] = CharClass::UNDERSCORE;

    return true;
}

// Get the character class of a given char
CharClass Scanner::getClass(char c)
{
    CharClass type = ascii_mapping[c];
    return type;
}

// Check if ch is a valid identifier character
bool Scanner::isValidIdentifier(char ch)
{
    CharClass cls = getClass(ch);
    return cls == CharClass::LETTER 
            || cls == CharClass::DIGIT 
            || cls == CharClass::UNDERSCORE;

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

//void Scanner::consume(const char* until)
//{
//    char ch;
//    bool checking = false;
//    int idx = 0;
//    while (input_file.get(ch))
//    {
//        if (!until[idx]) 
//        {
//            // Hit null terminator
//            input_file.putback(ch);
//            return;
//        }
//        if (ch == until[idx])
//        {
//            checking = true;
//            idx++;
//        }
//        else if (checking)
//        {
//            checking = false;
//            idx = 0;
//        }
//    }
//    // TODO: Handle I/O Error
//}

// Consume all leading whitespace and comments first
void Scanner::consumeWhitespaceAndComments()
{
    // Buffer
    char ch;
    while ((ch = input_file.peek()) 
            && ((getClass(ch) == CharClass::WHITESPACE) || ch == '/'))
    {
        if (ch == '/')
        {
            // First, consume the / so we can peek the next char 
            //  (ch will still be set to / in case the next char isn't / or *
            //  so division will still be parsed)
            input_file.get();
            char next = input_file.peek();
            // TODO: Remove comments
            if (next == '/')
            {
                // Consume line comment
                while (input_file.get() != '\n') {}
            }
            else if (next == '*')
            {
                input_file.get();

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
                }
            }
            else break;
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
    // The token to be returned; defaults to an identifier with no val
    Token token;
    token.type = TokenType::UNKNOWN;

    // Stores next char read from file
    char ch;

    consumeWhitespaceAndComments();

    // Store next char of file in ch
    input_file.get(ch);

    // Check for EOF
    if (input_file.eof())
    {
        token.type = TokenType::FILE_END;
        return token;
    }

    // Main switch to get token type (and value if necessary)
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
        // String token
        token.type = TokenType::STRING;
        // Need the extra scope level because k is defined in a case
        {
            int k = 0;
            while (input_file.get(ch) && ch != '"')
            //while ((ch = input_file.get()) != '"')
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
        input_file.get(ch);
        // TODO: Check ch for validity
        token.val.char_value = ch;
        ch = input_file.peek();
        if (ch != '\'')
        {
            // TODO: error: single quote containing more than one char
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
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '0':
        token.type = TokenType::INTEGER;
        token.val.int_value = (int)(ch - '0');
        while (input_file.get(ch))
        {
            if (ch == '.')
            {
                token.type = TokenType::FLOAT;
            }
            else if (getClass(ch) != CharClass::DIGIT) 
            {
                input_file.unget();
                break;
            }
            
            // TODO: What's a better way to do this? Is it easier to make
            //  a string first and call a to int method? Or what
            token.val.int_value = 10 * token.val.int_value + (int)(ch - '0');
            // TODO: deal with floats
        }
        break;
    default:
        // TODO: Turn this into a bunch of cases for each letter?
        // Identifiers/Reserved words must all start with a letter
        if (getClass(ch) == CharClass::LETTER)
        {
            int k;
            for (k = 0; k < MAX_STRING_LEN, isValidIdentifier(ch); k++)
            {
                token.val.string_value[k] = toupper(ch);
                input_file.get(ch);
            }
            // Put back most recently read char
            input_file.unget();
            // Null terminate the string
            token.val.string_value[k] = 0;

            // Check whether this is a reserved word or identifier
            token.type = getWordTokenType(token.val.string_value);
        }
        break;
    }

    // TODO: Check for file errors
    
    if (token.type == TokenType::UNKNOWN)
    {
        // TODO: Error, unknown char ch
        token.val.char_value = ch;
    }

    // DEBUG

    init_debug();
    std::cout << "LINE: " << line_number << " TOKEN: " << debug_typemap[token.type] << std::endl;

    // END DEBUG

    return token;
}

Scanner::~Scanner() { input_file.close(); }


