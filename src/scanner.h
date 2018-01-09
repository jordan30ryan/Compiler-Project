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
};
