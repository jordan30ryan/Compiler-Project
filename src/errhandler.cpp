#include "errhandler.h"

void ErrHandler::reportError(std::string message)
{
    std::cerr << "\033[31mError\033[0m: " <<  message << '\n';
    errors++;
}

void ErrHandler::reportError(std::string message, int line_num)
{
    std::cerr << "\033[31mError\033[0m (line " << line_num << "): " <<  message << '\n';
    errors++;
}

void ErrHandler::reportWarning(std::string message)
{
    std::cerr << "\033[33mWarning\033[0m: " << message << '\n';
    warnings++;
}

void ErrHandler::reportWarning(std::string message, int line_num)
{
    std::cerr << "\033[33mWarning\033[0m (line " << line_num << "): " << message << '\n';
    warnings++;
}

