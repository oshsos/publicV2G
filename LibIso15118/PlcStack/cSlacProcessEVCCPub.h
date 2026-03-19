#ifndef CSLACPROCESSEVCCPUB_H
#define CSLACPROCESSEVCCPUB_H

#include <string>

#include "../../LibCpp/cTimer.h"
#include "cPlcStackPub.h"
#include "cSlacPub.h"

namespace Iso15118
{

class cSlacProcessEVCCPub
{
public:
    cSlacProcessEVCCPub(cPlcStackPub* pPlcStack);
    virtual ~cSlacProcessEVCCPub();
    void clear();

    bool operate(bool start = true);
    inline bool isInitialized() {return state>=enSlacState::Idle;};
    inline enSlacState getState() {return state;};

public:
    LibCpp::cDebug  dbg;
    int             AAGaverage;
private:
    cPlcStackPub*   pPlcStack;
    enSlacState     state;
    bool            enteredState;

    LibCpp::cTimer  pauseTimer;                         ///< Timer to control pause between multiple sending of start attenuation and sound messages.
    LibCpp::cTimer  timer;
    //int             slacParamCnt;
    uint64_t        nextSlacParamTime;
    uint64_t        totalTimeout;
    uint64_t        settleEndTime;
};

};
#endif
