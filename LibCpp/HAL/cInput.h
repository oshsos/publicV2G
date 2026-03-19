#ifndef CINPUT_H
#define CINPUT_H

namespace LibCpp
{

class cInput
{
public:
    cInput();

    virtual bool get();

public:
    bool state;
};

}

#endif
