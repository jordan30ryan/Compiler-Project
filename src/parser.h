#pragma once
#include "scanner.h"

class Parser
{
public:
    Parser(Scanner* scan);
private:
    Scanner* scanner;
};

