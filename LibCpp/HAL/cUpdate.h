#ifndef CUPDATE_H
#define CUPDATE_H

#include <vector>

#include "cCallback.h"

namespace LibCpp
{
    class cUpdate;

    class iUpdate
    {
    public:
        virtual void onUpdate(cUpdate* source, int typeIndex) = 0;
    };

    class cUpdate : public cCallback<iUpdate>
    {
    protected:
        virtual void runUpdate(int typeIndex = 0);                                       ///> Calls all registered callback instances.
    };
}

#endif
