#include <iostream>

extern "C" void PUTINTEGER(int val)
{
    std::cout << val;
}
