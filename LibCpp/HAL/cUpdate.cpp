#include "cUpdate.h"

using namespace std;
using namespace LibCpp;

/**
 * @brief Calls all registered callback instances.
 */
void cUpdate::runUpdate(int typeIndex)
{
    for(iUpdate* pInst : callbacks)
        pInst->onUpdate(this, typeIndex);
}
