#include <fstream>
#include "token.h"

class Scanner
{
public:
    bool init(char* filename);
    Token getToken();
    ~Scanner();
private:
    std::ifstream input_file;
    int line_number;
    CharClass ascii_mapping[128] = {TokenType::UNDEFINED};

    CharClass getClass(char c);
};
