#ifndef TYPES_H
#define TYPES_H

#include <string>

namespace Iso15118
{

enum class enStackType
{
    evcc,
    secc
};

std::string enStackType_toString(enStackType stackType);

}

#endif
