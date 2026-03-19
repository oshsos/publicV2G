#include <conio.h>

#include "../Tools.h"

#include "cWinConsoleInput.h"

using namespace LibCpp;

cWinConsoleInput::cWinConsoleInput()
{
    endSignalKey = '\n';
    endLineChar = '\n';
    stateReadLine = readLineState::READY;

    SetConsoleOutputCP(CP_UTF8);
    hin=CreateFileA("CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
}

bool cWinConsoleInput::hasKey()
{
    if (_kbhit())
    {
        inputKey = getch();
        if (stateReadLine == readLineState::STARTED)
        {
            if ((endLineChar == '\n' && (inputKey == endLineChar || inputKey == '\r')) ||
                (inputKey == endLineChar))
            {
                printf_flush("\n");
                stateReadLine = readLineState::PENDING;
                inputKey = endSignalKey;
            }
            else
            {
                printf_flush("%c", inputKey);
                if (inputKey == 8)
                    printf_flush(" %c", inputKey);
                readLineBuffer += inputKey;
                inputKey = 0;
            }
        }
    }
    else
        inputKey = 0;
    return inputKey != 0;
}

/**
 * @brief cWinConsoleInput::readLine
\code
while (1)
{
    if (console.hasKey())
    {
        if (input == 'm')
        {
            cWinConsoleInput::readLineState res = console.readLine('m');
            if (res == cWinConsoleInput::readLineState::STARTED)
                printf_flush("Type message:\n");
            if (res == cWinConsoleInput::readLineState::READY)
                string message = console.getLine();
        }
    }
}
\endcode

 * @param endSignalKey
 * @return
 */
cWinConsoleInput::readLineState cWinConsoleInput::readLine(char endSignalKey)
{
    this->endSignalKey = endSignalKey;
    if (stateReadLine == readLineState::READY)
    {
        readLineBuffer = "";
        stateReadLine = readLineState::STARTED;
        return readLineState::STARTED;
    }
    else if (stateReadLine == readLineState::PENDING)
    {
        stateReadLine = readLineState::READY;
        return readLineState::READY;;
    }
    return readLineState::PENDING;
}

std::string cWinConsoleInput::getLine()
{
    return readLineBuffer;
}
