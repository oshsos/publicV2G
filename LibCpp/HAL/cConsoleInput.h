/**
 * @brief cWinConsoleInput::readLine
\code
cWinConsoleInput console;

while (1)
{
    if (console.hasKey())
    {
        if (input == 'm')
        {
            cConsoleInput::readLineState res = console.readLine('m');
            if (res == cConsoleInput::readLineState::STARTED)
                printf_flush("Type message:\n");
            if (res == cConsoleInput::readLineState::READY)
                string message = console.getLine();
        }
    }
}
\endcode
 */

#ifndef CCONSOLEINPUT_H
#define CCONSOLEINPUT_H

#include <string>

namespace LibCpp
{

class cConsoleInput
{
public:
    enum class readLineState
    {
        STARTED,
        PENDING,
        READY
    };

public:
    cConsoleInput();
    virtual bool            hasKey() = 0;
    inline char             getKey() {return inputKey;};

    virtual readLineState   readLine(char endSignalKey = '\n') = 0;
    virtual std::string     getLine() = 0;

protected:
    char inputKey;
};

}
#endif
