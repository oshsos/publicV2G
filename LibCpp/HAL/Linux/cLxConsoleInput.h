#ifndef CLXCONSOLEINPUT_H
#define CLXCONSOLEINPUT_H

#include <string>

#include "../cConsoleInput.h"

namespace LibCpp
{

class cLxConsoleInput : public cConsoleInput
{
public:
    cLxConsoleInput();
    bool            hasKey();

    readLineState   readLine(char endSignalKey = '\n');
    std::string     getLine();

protected:
    char            endLineChar;
    char            endSignalKey;
    readLineState   stateReadLine;
    std::string     readLineBuffer;
};

}
#endif
