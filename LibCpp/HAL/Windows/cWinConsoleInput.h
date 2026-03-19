/**

\code
SOURCES += \
        LibCpp/HAL/Linux/HW_Tools.cpp \
        LibCpp/HAL/Linux/cLxConsoleInput.cpp \
        LibCpp/HAL/Linux/conio.cpp \
        LibCpp/HAL/Tools.cpp \
        LibCpp/HAL/cDebug.cpp \
        main.cpp

HEADERS += \
    LibCpp/HAL/Linux/cLxConsoleInput.h \
    LibCpp/HAL/Linux/conio.h \
    LibCpp/HAL/Tools.h \
    LibCpp/HAL/cDebug.h
\endcode

\code

#include <iostream>
#include <unistd.h>

#include "LibCpp/HAL/cDebug.h"
#include "LibCpp/HAL/Tools.h"
#include "LibCpp/HAL/Linux/cLxConsoleInput.h"

using namespace std;
using namespace LibCpp;

cDebug dbg("main", enDebugLevel_Debug);
cLxConsoleInput  console;

void printHelp()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'s' to do whatever\n");
}

int main()
{
    dbg.setInstanceName("main");

    printf_flush("Console example program\n");
    printHelp();

    while (1)
    {
        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelp();
            }
            if (input == 's')
            {
            }
        }
        usleep(10000);
    }
    return 0;
}
\endcode
*/

#ifndef CWINCONSOLEINPUT_H
#define CWINCONSOLEINPUT_H

#include <windows.h>
#include <string>

#include "../cConsoleInput.h"

namespace LibCpp
{

class cWinConsoleInput : public cConsoleInput
{
public:
    cWinConsoleInput();
    bool hasKey();

    inline char getKey() {return inputKey;};
    readLineState readLine(char endSignalKey = '\n');
    std::string getLine();

private:
    HANDLE hin;
    INPUT_RECORD ir;
    char inputKey;
    char preInput;

    char endLineChar;
    char endSignalKey;
    readLineState stateReadLine;
    std::string readLineBuffer;
};

}
#endif
