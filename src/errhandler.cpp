#include "errhandler.h"
#include <iostream>

void ErrHandler::reportError(std::string message)
{
    std::cerr << "Error: " <<  message << '\n';
    has_errors = true;
}

void ErrHandler::reportError(std::string message, int line_num)
{
    std::cerr << "Error (line " << line_num << "): " <<  message << '\n';
    has_errors = true;
}

void ErrHandler::reportWarning(std::string message)
{
    std::cout << "Warning: " << message << '\n';
}


