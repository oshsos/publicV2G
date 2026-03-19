#include "types.h"

using namespace std;

string Iso15118::enStackType_toString(enStackType stackType)
{
    switch (stackType)
    {
    case enStackType::evcc: return "EVCC";
    case enStackType::secc: return "SECC";
    default: return "<undefined>";
    }
}
