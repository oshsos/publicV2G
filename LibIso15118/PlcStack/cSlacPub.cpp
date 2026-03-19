#include "cSlacPub.h"

std::string Iso15118::enSlacState_toString(enSlacState state)
{
    switch (state)
    {
    case enSlacState::awaitGetSwVer: return "awaitGetSwVer";
    case enSlacState::awaitSetup: return "awaitSetup";
    case enSlacState::Idle: return "Idle";
    case enSlacState::awaitEstablishing: return "awaitEstablishing";
    case enSlacState::awaitStartAttenChar: return "awaitStartAttenChar";
    case enSlacState::awaitSounding: return "awaitSounding";
    case enSlacState::awaitAttenChar: return "awaitAttenChar";
    case enSlacState::awaitMatch: return "awaitMatch";
    case enSlacState::awaitSetKey: return "awaitSetKey";
    case enSlacState::awaitSettleTime: return "awaitSettleTime";
    case enSlacState::Failed: return "Failed";
    case enSlacState::Matched: return "Finished";
    }
    return "<undefined>";
}
