#ifndef CSUPERWISEDOUTPUT_H
#define CSUPERWISEDOUTPUT_H

#include "cOutput.h"

namespace LibCpp
{

class cSuperwisedOutput : public cOutput
{
public:
    cSuperwisedOutput(cOutput* pAttachedOutput = nullptr);

    void setOutput(cOutput* pAttachedOutput);

    virtual void set(bool on = true);
    virtual bool get();
    virtual bool getDesired();

protected:
    cOutput*    pAttacedOutput;
};

}

#endif
