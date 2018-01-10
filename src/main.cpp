#include "scanner.h"
#include <vector>

void reportError(const char* message)
{

}

void reportWarning(const char* message)
{

}

std::vector<Token> scan(char* filename) 
{
    // Gets a vector of tokens from the scanner
    Scanner scanner;
    scanner.init(filename);
    Token next_token;

    std::vector<Token> tokens;
    while ((next_token = scanner.getToken()).type != TokenType::FILE_END)
    {
        tokens.push_back(next_token);
    }
    return tokens;
}

int main(int argc, char** argv)
{
    if (argc < 2) 
    {
        reportError("No filename provided.");
        return -1;
    }

    scan(argv[1]);
    //TODO Parser
    //TODO Type checking
    //TODO Code generation
    
    return 0;
}

