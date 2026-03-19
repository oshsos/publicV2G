#define SLAC_SETTLE SLAC_SETTLETIME + 100
#define SLAC_TOTAL_TIMEOUT 60000

//#include "../../LibCpp/HAL/Windows/cWinTcpServer.h"
#include "cSlacProcessSECCPub.h"
#include "../../LibCpp/HAL/Tools.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

//cWinTcpServer server;
//cConnectedClient* pClient = nullptr;

cSlacProcessSECCPub::cSlacProcessSECCPub(cPlcStackPub* pPlcStack) :
    dbg("cSlacProcessPub")
{
    this->pPlcStack = pPlcStack;
    totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);
    state = enSlacState::awaitGetSwVer;
    enteredState = true;
}

cSlacProcessSECCPub::~cSlacProcessSECCPub()
{
}

bool cSlacProcessSECCPub::operate(bool start)
{
    cDebug dbg("operate", &this->dbg);

    //int len = pPlcStack->receiveBuffer();

    // bool finished = false;
    enSlacState nextState = state;

    if (timer.passed(totalTimeout) && !(state == enSlacState::awaitGetSwVer || state == enSlacState::Idle || state == enSlacState::awaitGetSwVer ||  state == enSlacState::awaitEstablishing || state == enSlacState::Matched || state == enSlacState::Failed))
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Slac process failed with total timeout of %i ms!", SLAC_TOTAL_TIMEOUT);
        nextState = enSlacState::Failed;
    }
    else
    {
        switch (state)
        {
        case enSlacState::awaitGetSwVer:
        {
            if (enteredState)
            {
                pPlcStack->send_GET_SW_VER_REQ();
            }
            if (pPlcStack->lastEvaluationResult == VS_SW_VER)
            {
                nextState = enSlacState::Idle;
            }
        }; break;
        case enSlacState::awaitSetup:
        {
            if (enteredState)
            {
                pPlcStack->send_GET_SW_VER_REQ();
            }
            if (pPlcStack->lastEvaluationResult == VS_SW_VER)
            {
                nextState = enSlacState::Idle;
            }
        }; break;
        case enSlacState::Idle:
        {
            if (enteredState)
            {
                pPlcStack->clear();
            }
            if (start)
            {
                totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);
                nextState = enSlacState::awaitEstablishing;
            }
        }; break;
        case enSlacState::awaitEstablishing:
        {
            if (pPlcStack->lastEvaluationResult == CM_SLAC_PARAM)
            {
                pPlcStack->send_CM_SLAC_PARAM_CNF();
                totalTimeout = timer.getAhead(SLAC_TOTAL_TIMEOUT);
                nextState = enSlacState::awaitStartAttenChar;
            }
        }; break;
        case enSlacState::awaitStartAttenChar:
        {
            if (pPlcStack->lastEvaluationResult == CM_START_ATTEN_CHAR)
            {
                // pPlcStack->send_CM_START_ATTEN_CHAR_RSP();
                nextState = enSlacState::awaitSounding;
            }
        }; break;
        case enSlacState::awaitSounding:
        {
            if (enteredState)
            {
                soundEndTime = timer.getAhead(1500);
            }
    //        if (pPlcStack->lastEvaluationResult == CM_START_ATTEN_CHAR)
    //        {
    //            pPlcStack->send_CM_START_ATTEN_CHAR_RSP();
    //        }
            if (pPlcStack->soundsComplete())
            {
                pPlcStack->send_CM_ATTEN_CHAR_IND();
                nextState = enSlacState::awaitAttenChar;
            }
            else if (timer.passed(soundEndTime))
            {
                dbg.printf(LibCpp::enDebugLevel_Debug, "Sounds interrupted!");
                pPlcStack->send_CM_ATTEN_CHAR_IND();
                nextState = enSlacState::awaitAttenChar;
            }
    //        else
    //            dbg.printf(LibCpp::enDebugLevel_Debug, "Sounds uncomplete.");
        }; break;
        case enSlacState::awaitAttenChar:
        {
            if (pPlcStack->lastEvaluationResult == CM_ATTEN_CHAR)
            {
//                unsigned int sum = 0;
//                int AAGaverage = 0;
//                for (unsigned int i=0; i< pPlcStack->values.NumGroups; i++)
//                    sum += pPlcStack->values.AAG[i];
//                if (pPlcStack->values.NumGroups)
//                    AAGaverage = sum / pPlcStack->values.NumGroups;
//                dbg.printf(enDebugLevel_Info, "Attenuation 0x%02x [max: 0x%2x] (%s)", AAGaverage, SLAC_LIMIT, ByteArrayToString(pPlcStack->values.AAG, pPlcStack->values.NumGroups).c_str());
                nextState = enSlacState::awaitMatch;
            }
         }; break;
        case enSlacState::awaitMatch:
        {
            if (pPlcStack->lastEvaluationResult == CM_SLAC_MATCH)
            {
                pPlcStack->send_CM_SLAC_MATCH_CNF();
                pPlcStack->send_CM_SET_KEY_REQ();
                nextState = enSlacState::awaitSetKey;
            }
        }; break;
        case enSlacState::awaitSetKey:
        {
            if (pPlcStack->lastEvaluationResult == CM_SET_KEY)
            {
                if (pPlcStack->connectionSuccessful())
                    nextState = enSlacState::Matched;
                else
                    nextState = enSlacState::awaitEstablishing;
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
        {
            if (!start)
                nextState = enSlacState::Idle;
        }; break;
        case enSlacState::Matched:
        {
            if (!start)
                nextState = enSlacState::Idle;
        }; break;
        }
    }
    pPlcStack->lastEvaluationResult = 0xFFFF;       // Confirm processed message being previously received and evaluated.
    bool finished = (state == enSlacState::Matched || state == enSlacState::Failed);
    enteredState = (nextState!=state);
    state = nextState;

    if (enteredState)
    {
        dbg.printf(LibCpp::enDebugLevel_Info, "Entered state %s", enSlacState_toString(state).c_str());
    }

    return finished;
}
