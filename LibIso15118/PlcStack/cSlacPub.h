#ifndef CSLACPUB_H
#define CSLACPUB_H

#include <string>

namespace Iso15118
{

enum class enSlacState
{
    awaitGetSwVer,
    awaitSetup,
    Idle,
    awaitEstablishing,
    awaitStartAttenChar,
    awaitSounding,
    awaitAttenChar,
    awaitMatch,
    awaitSetKey,
    awaitSettleTime,
    Failed,
    Matched
};

std::string enSlacState_toString(enSlacState state);

}
#endif
