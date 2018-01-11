#include <fstream>
#include "token.h"

class Scanner
{
public:
    /*
    Sets up the scanner to read from a file.
    Initializes variables in the class.

    filename - name of the file to read the program from

    returns - whether the scanner was successfully initialized (mainly whether the file was initialized for reading)
    */
    bool init(const char* filename);

    Token getToken();
    ~Scanner();
private:
    std::ifstream input_file;
    int line_number;
    CharClass ascii_mapping[128] = {CharClass::UNDEFINED};

    CharClass getClass(char c);

    void consume(const char* until);
};

