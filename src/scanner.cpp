#include "scanner.h"

// TODO: This needs to go somewhere else
std::unordered_map<std::string, SymTableEntry> global_symbols;

Scanner::Scanner(ErrHandler* handler) : err_handler(handler) {}

bool Scanner::init(const char* filename)
{
    input_file.open(filename, std::ifstream::in);
    if (!input_file.is_open() || input_file.bad()) return false;
    
    line_number = 1;

    // Init reserved words table
    global_symbols.insert({"IN", SymTableEntry(TokenType::RS_IN)});
    global_symbols.insert({"OUT", SymTableEntry(TokenType::RS_OUT)});
    global_symbols.insert({"INOUT", SymTableEntry(TokenType::RS_INOUT)});
    global_symbols.insert({"PROGRAM", SymTableEntry(TokenType::RS_PROGRAM)});
    global_symbols.insert({"IS", SymTableEntry(TokenType::RS_IS)});
    global_symbols.insert({"BEGIN", SymTableEntry(TokenType::RS_BEGIN)});
    global_symbols.insert({"END", SymTableEntry(TokenType::RS_END)});
    global_symbols.insert({"GLOBAL", SymTableEntry(TokenType::RS_GLOBAL)});
    global_symbols.insert({"PROCEDURE", SymTableEntry(TokenType::RS_PROCEDURE)});
    global_symbols.insert({"STRING", SymTableEntry(TokenType::RS_STRING)});
    global_symbols.insert({"CHAR", SymTableEntry(TokenType::RS_CHAR)});
    global_symbols.insert({"INTEGER", SymTableEntry(TokenType::RS_INTEGER)});
    global_symbols.insert({"FLOAT", SymTableEntry(TokenType::RS_FLOAT)});
    global_symbols.insert({"BOOL", SymTableEntry(TokenType::RS_BOOL)});
    global_symbols.insert({"IF", SymTableEntry(TokenType::RS_IF)});
    global_symbols.insert({"THEN", SymTableEntry(TokenType::RS_THEN)});
    global_symbols.insert({"ELSE", SymTableEntry(TokenType::RS_ELSE)});
    global_symbols.insert({"FOR", SymTableEntry(TokenType::RS_FOR)});
    global_symbols.insert({"RETURN", SymTableEntry(TokenType::RS_RETURN)});
    global_symbols.insert({"TRUE", SymTableEntry(TokenType::RS_TRUE)});
    global_symbols.insert({"FALSE", SymTableEntry(TokenType::RS_FALSE)});
    global_symbols.insert({"NOT", SymTableEntry(TokenType::RS_NOT)});

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
TokenType Scanner::getWordTokenType(std::string str)
{
    TokenType type;
    try
    {
        type = global_symbols.at(str).type;
    }
    catch (const std::out_of_range& oor) 
    {
        // str not in the map; just an identifier
        return TokenType::IDENTIFIER;
    }
    return type;
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

    // For counting loops (as to avoid cross init in cases)
    int k;

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

    // Initial ch value 
    token.val.char_value = ch;

    // Main switch to get token type (and value if necessary)
    switch (ascii_mapping[ch])
    {
    case CharClass::WHITESPACE:
        // Something in consumeWhitespace... is not working correctly...
        break;
    case CharClass::LETTER:
        // Identifiers/Reserved words must all start with a letter
        while (isValidInIdentifier(ch))
        {
            // append char to str
            token.val.string_value.push_back((char)toupper(ch));
            input_file.get(ch);
        }
        // Put back most recently read char (it wasn't valid in an identifier)
        input_file.unget();

        // Check whether this is a reserved word or identifier
        token.type = getWordTokenType(token.val.string_value);
        if (token.type == TokenType::IDENTIFIER)
        {
            // Add to sym table
            global_symbols[token.val.string_value];
        }
        break;
    case CharClass::DIGIT:
        // Extra scope level needed becuase of variables defined in this case
        {
            token.type = TokenType::INTEGER;

            token.val.int_value = (int)(ch - '0');

            bool is_fractional_part = false;
            double fract_mult = 0.1;

            while (input_file.get(ch))
            {
                if (ch == '.')
                {
                    token.type = TokenType::FLOAT;
                    token.val.float_value = token.val.int_value;
                    is_fractional_part = true;
                    continue;
                }
                else if (ch == '_') continue;
                else if (ascii_mapping[ch] != CharClass::DIGIT) 
                {
                    input_file.unget();
                    break;
                }
                
                // Is there a better way to do this?
                if (is_fractional_part)
                {
                    token.val.float_value += fract_mult * (ch - '0');
                    fract_mult *= 0.1;
                }
                else 
                {
                    token.val.int_value = 10 * token.val.int_value + (int)(ch - '0');
                }
            }
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
            // String token
            token.type = TokenType::STRING;

            k = 0;
            while (input_file.get(ch) && ch != '"')
            {
                if (!isValidInString(ch))
                {
                    std::ostringstream stream;
                    stream << "Char not valid in a string: " << ch;
                    err_handler->reportError(stream.str(), line_number);
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
            break;
        case '\'':
            token.type = TokenType::CHAR;
            input_file.get(ch);
            if (!isValidChar(ch))
            {
                std::ostringstream stream;
                stream << "Not a valid char literal: " << ch;
                err_handler->reportError(stream.str(), line_number);
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
        std::ostringstream stream;
        stream << "Unknown token: " << ch;
        err_handler->reportError(stream.str(), line_number);
    }

    return token;
}

Scanner::~Scanner() { input_file.close(); }

