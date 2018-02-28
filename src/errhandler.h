#pragma once
#include <string>
#include <iostream>

class ErrHandler
{
public:
    void reportError(std::string message);
    void reportError(std::string message, int line_num);
    void reportWarning(std::string message);
    void reportWarning(std::string message, int line_num);
    int errors = 0;
    int warnings = 0;
};

