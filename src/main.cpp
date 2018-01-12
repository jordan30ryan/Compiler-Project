#include "scanner.h"
#include <vector>
#include <iostream>

void reportError(const char* message)
{
    std::cerr << message << '\n';
}

void reportWarning(const char* message)
{
    std::cout << message << '\n';
}

// Gets a vector of tokens from the scanner
std::vector<Token> scan(char* filename) 
{
    Scanner scanner;
    bool init_success = scanner.init(filename);
    if (!init_success)
    {
        reportError("Scanner initialization failed. Ensure the input file is valid.");
    }

    Token next_token;
    std::vector<Token> tokens;
    while ((next_token = scanner.getToken()).type != TokenType::FILE_END)
    {
        tokens.push_back(next_token);
    }
    return tokens;
}

/*
Return codes
1 - No filename given
2 - Empty file or I/O error (no tokens given by scanner)
*/
int main(int argc, char** argv)
{
    if (argc < 2) 
    {
        reportError("No filename provided.");
        return 1;
    }

    std::vector<Token> tokens = scan(argv[1]);
    if (tokens.size() == 0) return 2;

    // DEBUG

    init_debug();
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        std::cout << debug_typemap[it->type] << '\t';
        if (it->type == TokenType::IDENTIFIER)
        {
            std::cout << it->val.string_value;
        }
        if (it->type == TokenType::INTEGER)
        {
            std::cout << it->val.int_value;
        }
        std::cout << std::endl;
        //std::cout << '\t' << it->val.string_value << '\n';
    }

    // DEBUG END

    //TODO Parse the tokens
    //TODO Type checking
    //TODO Code generation
    
    return 0;
}

