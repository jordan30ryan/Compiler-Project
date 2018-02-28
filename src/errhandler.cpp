#include "errhandler.h"

void ErrHandler::reportError(std::string message)
{
    std::cerr << "Error: " <<  message << '\n';
    errors++;
}

void ErrHandler::reportError(std::string message, int line_num)
{
    std::cerr << "Error (line " << line_num << "): " <<  message << '\n';
    errors++;
}

void ErrHandler::reportWarning(std::string message)
{
    std::cout << "Warning: " << message << '\n';
    warnings++;
}

void ErrHandler::reportWarning(std::string message, int line_num)
{
    std::cout << "Warning (line " << line_num << "): " << message << '\n';
    warnings++;
}

