#pragma once
#include "scanner.h"

class Parser
{
public:
    Parser(Scanner* scan);
    void parse();
private:
    Scanner* scanner;
};

