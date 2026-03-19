#include "cOutput.h"

using namespace LibCpp;

cOutput::cOutput()
{
}

void cOutput::set(bool on)
{
    state = on;
}

bool cOutput::get()
{
    return state;
}
