#ifndef CSLACPROCESSSECC_H
#define CSLACPROCESSSECC_H

#include <string>
#include "cPlcStackPub.h"
#include "cSlacPub.h"
#include "../../LibCpp/cTimer.h"

namespace Iso15118
{

class cSlacProcessSECCPub
{
public:
    cSlacProcessSECCPub(cPlcStackPub* pPlcStack);
    virtual ~cSlacProcessSECCPub();

    bool operate(bool start = true);
    inline bool isInitialized() {return state>=enSlacState::Idle;};
    inline enSlacState getState() {return state;};

public:
    LibCpp::cDebug  dbg;
    cPlcStackPub*   pPlcStack;

private:
    enSlacState     state;
    bool            enteredState;

    LibCpp::cTimer  timer;
    uint64_t        soundEndTime;
    uint64_t        settleEndTime;
    uint64_t        totalTimeout;
};

}
#endif
