#define SLAC_PARAM_REQ_CYCLE 500            ///< Cycle time for SLAC parameter request messages in ms
#define SLAC_PARAM_REQ_CONTINUE_CYCLE 500   ///< Cycle time for SLAC parameter request while being in connection fault state
#define SLAC_PARAM_REQ_MIN 1                ///< Minimum numbers of emitted requests to give each potential charge point enough time to respond.
#define SLAC_PARAM_REQ_MAX 10               ///< Maximum numbers of emitted requests before entering the link connection fault state.
#define SLAC_REPEAT 3                       ///< Repeated sending of CM_START_ATTEN.IND and CM_SLAC_PARAM.REQ
#define SLAC_TOTAL_TIMEOUT 10000            ///< Timeout for the complete SLAC process to finish in case communicaton flow breaks
#define SLAC_SETTLE SLAC_SETTLETIME+100     ///< Settle time of slac process before allowing high level communication

#include "../../LibCpp/HAL/Tools.h"

#include "cSlacProcessEVCCPub.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

cSlacProcessEVCCPub::cSlacProcessEVCCPub(cPlcStackPub* pPlcStack) :
    dbg("cSlacProcessEVCCPub"),
    pauseTimer(SLAC_PAUSE)
{
    this->pPlcStack = pPlcStack;
    state = enSlacState::awaitGetSwVer;
    enteredState = true;
    // clear();
}

void cSlacProcessEVCCPub::clear()
{
    //slacParamCnt = 0;
    //nextSlacParamTime = timer.get();
    //totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);

    pPlcStack->clear();
//    memcpy(pPlcStack->values.NID, PEV_NID, SLAC_NID_LEN);
//    memcpy(pPlcStack->values.NMK, PEV_NMK, SLAC_NMK_LEN);
//    pPlcStack->send_CM_SET_KEY_REQ();
}

cSlacProcessEVCCPub::~cSlacProcessEVCCPub()
{
}

bool cSlacProcessEVCCPub::operate(bool start)
{
    cDebug dbg("operate", &this->dbg);

    //int len = pPlcStack->receiveBuffer();

    // bool finished = false;
    enSlacState nextState = state;
    if (enteredState)
    {
        dbg.printf(LibCpp::enDebugLevel_Info, "Entered state %s", enSlacState_toString(state).c_str());
    }

    if (timer.passed(totalTimeout) && !(state == enSlacState::awaitGetSwVer || state == enSlacState::Idle || state == enSlacState::Failed || state == enSlacState::Matched))
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Slac process failed with total timeout of %i ms!", SLAC_TOTAL_TIMEOUT);
        nextState = enSlacState::Failed;
    }
    else
    {
        switch (state)
        {
        case enSlacState::awaitGetSwVer:
        case enSlacState::awaitSetup:
            if (enteredState)
            {
                pPlcStack->send_GET_SW_VER_REQ();
            }
            if (pPlcStack->lastEvaluationResult == VS_SW_VER)
            {
                nextState = enSlacState::Idle;
            }
            break;
        case enSlacState::Idle:
            if (enteredState)
            {
                clear();
            }
            if (start)
            {
                totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);
                nextState = enSlacState::awaitEstablishing;
            }
            break;
        case enSlacState::awaitEstablishing:
            if (enteredState)
            {
                nextSlacParamTime = timer.get();
            }
            if (!start)
                nextState = enSlacState::Idle;
            if (pPlcStack->lastEvaluationResult == CM_SLAC_PARAM)
            {
                nextState = enSlacState::awaitStartAttenChar;
            }
            if (timer.passed(nextSlacParamTime))
            {
                //if (slacParamCnt) slacParamCnt--;
                nextSlacParamTime = timer.getAhead(SLAC_PARAM_REQ_CYCLE);
                totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);
                pPlcStack->send_CM_SLAC_PARAM_REQ();
            }
            break;
//        case enSlacState::awaitSlacParam:
//            if (enteredState)
//            {
//                pPlcStack->send_CM_SLAC_PARAM_REQ();
//            }
//            if (pPlcStack->lastEvaluationResult == CM_SLAC_PARAM)
//            {
//                nextState = enSlacState::awaitStartAttenChar;
//                nextState = enSlacState::Finished;
//            }
//            break;
        case enSlacState::awaitStartAttenChar:
            pPlcStack->send_CM_START_ATTEN_CHAR_IND();
            nextState = enSlacState::awaitSounding;
            break;
        case enSlacState::awaitSounding:
            if (enteredState)
            {
                pPlcStack->values.sounds = pPlcStack->values.NUM_SOUNDS;
                if (pPlcStack->values.sounds < 1) pPlcStack->values.sounds = 1;
                pauseTimer.start();
            }
            if (enteredState || (pPlcStack->values.sounds > 0 && pauseTimer.elapsed()))
            {
                pPlcStack->values.sounds--;
                pPlcStack->send_CM_MNBC_SOUND_IND(pPlcStack->values.sounds);
                pauseTimer.start();
            }
            if (pPlcStack->values.sounds == 0)
            {
                nextState = enSlacState::awaitAttenChar;
            }
            break;
        case enSlacState::awaitAttenChar:
            if (pPlcStack->lastEvaluationResult == CM_ATTEN_CHAR)
            {
                pPlcStack->send_CM_ATTEN_CHAR_RSP();
                unsigned int sum = 0;
                for (unsigned int i=0; i< pPlcStack->values.NumGroups; i++)
                    sum += pPlcStack->values.AAG[i];
                if (pPlcStack->values.NumGroups)
                    AAGaverage = sum / pPlcStack->values.NumGroups;
                dbg.printf(enDebugLevel_Info, "Attenuation 0x%2x [max: 0x%2x] (%s)", AAGaverage, SLAC_LIMIT, ByteArrayToString(pPlcStack->values.AAG, pPlcStack->values.NumGroups).c_str());
                nextState = enSlacState::awaitMatch;
            }
            break;
        case enSlacState::awaitMatch:
            if (enteredState)
            {
                pPlcStack->send_CM_SLAC_MATCH_REQ();
            }
            if (pPlcStack->lastEvaluationResult == CM_SLAC_MATCH)
            {
//                pPlcStack->send_CM_SLAC_MATCH_CNF();
//                pPlcStack->send_CM_SET_KEY_REQ();
                nextState = enSlacState::awaitSetKey;
            }
            break;
        case enSlacState::awaitSetKey:
        {
            if (enteredState)
            {
                pPlcStack->send_CM_SET_KEY_REQ();
            }
            if (pPlcStack->lastEvaluationResult == CM_SET_KEY)
            {
                nextState = enSlacState::awaitSettleTime;
            }
        }; break;
        case enSlacState::awaitSettleTime:
        {
            if (enteredState)
                settleEndTime = timer.getAhead(SLAC_SETTLE);
            if (timer.passed(settleEndTime))
                nextState = enSlacState::Matched;
        }; break;
        case enSlacState::Failed:
            if (!start)
                nextState = enSlacState::Idle;
            break;
        case enSlacState::Matched:
            if (!start)
                nextState = enSlacState::Idle;
            break;
        }
    }

    pPlcStack->lastEvaluationResult = 0xFFFF;       // Confirm processed message being previously received and evaluated.
    bool finished = (state == enSlacState::Matched || state == enSlacState::Failed);
    enteredState = (nextState!=state);
    state = nextState;

    // if (len) pPlcStack->receiveAcknowledge();

    return finished;
}
