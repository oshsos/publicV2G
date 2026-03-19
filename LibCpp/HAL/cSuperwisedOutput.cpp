#include "cSuperwisedOutput.h"

using namespace LibCpp;

cSuperwisedOutput::cSuperwisedOutput(cOutput* pAttachedOutput) :
    pAttacedOutput(pAttachedOutput)
{
}

void cSuperwisedOutput::setOutput(cOutput* pAttachedOutput)
{
    this->pAttacedOutput = pAttachedOutput;
}

void cSuperwisedOutput::set(bool on)
{
    if (pAttacedOutput)
        pAttacedOutput->set(on);
    else
        state = on;
}

bool cSuperwisedOutput::get()
{
    if (pAttacedOutput)
        return pAttacedOutput->get();
    else
        return state;
}

bool cSuperwisedOutput::getDesired()
{
    if (pAttacedOutput)
        return pAttacedOutput->get();
    else
        return state;
}
