#pragma once
#include <string>
#include <iostream>

class ErrHandler
{
public:
    void reportError(std::string message);
    void reportError(std::string message, int line_num);
    void reportWarning(std::string message);
    int errors;
};

