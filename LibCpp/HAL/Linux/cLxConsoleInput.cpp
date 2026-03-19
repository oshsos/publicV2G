#include "../Tools.h"
#include "conio.h"
#include "cLxConsoleInput.h"

using namespace LibCpp;

cLxConsoleInput::cLxConsoleInput()
{
    endSignalKey = '\n';
    endLineChar = '\n';
    stateReadLine = readLineState::READY;
}

bool cLxConsoleInput::hasKey()
{
    if (_kbhit())
    {
        inputKey = getch();

        if (stateReadLine == readLineState::STARTED)
        {
            if (inputKey == 127) // back space
            {
                readLineBuffer.pop_back();
                LibCpp::printf_flush("%c %c%c %c%c %c", 8, 8, 8, 8, 8, 8);
                inputKey = 0;
            }
            else if ((endLineChar == '\n' && (inputKey == endLineChar || inputKey == '\r')) ||
                     (inputKey == endLineChar))
            {
                // printf("\n");
                stateReadLine = readLineState::PENDING;
                inputKey = endSignalKey;
            }
            else
            {
                // printf("%c", inputKey);
                readLineBuffer += inputKey;
                inputKey = 0;
            }
        }
    }
    else
        inputKey = 0;
    return inputKey != 0;
}

cConsoleInput::readLineState cLxConsoleInput::readLine(char endSignalKey)
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

std::string cLxConsoleInput::getLine()
{
    return readLineBuffer;
}
