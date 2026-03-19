#include "cWinConsoleInput.h"

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
    DWORD num=0;
    inputKey = 0;
    PeekConsoleInput(hin, &ir, 1, &num);
    if (num)
    {
        ReadConsoleInputA(hin, &ir, 1, &num);
        if (ir.EventType == KEY_EVENT)
        {
            if (ir.Event.KeyEvent.bKeyDown)
            {
                preInput = ir.Event.KeyEvent.uChar.AsciiChar;
            }
            else
            {
                if (stateReadLine == readLineState::STARTED)
                {
                    if (preInput == 8) // back space
                    {
                        readLineBuffer.pop_back();
                        printf("%c %c", preInput, preInput);
                        preInput = 0;
                    }
                    else if ((endLineChar == '\n' && (preInput == endLineChar || preInput == '\r')) ||
                        (preInput == endLineChar))
                    {
                        printf("\n");
                        stateReadLine = readLineState::PENDING;
                        preInput = endSignalKey;
                    }
                    else
                    {
                        printf("%c", preInput);
                        readLineBuffer += preInput;
                        preInput = 0;
                    }
                }
                inputKey = preInput;
            }
        }
    }
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
