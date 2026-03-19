#ifndef Y2038CALENDAR_H
#define Y2038CALENDAR_H

#include "cTime.h"

namespace LibCpp
{
    template <typename T, typename uT> void unsignedModulo(T value, uT modulo, T& divisor,  uT& remainder);
    extern void (*unsignedModulo_i64)(int64_t, uint64_t, int64_t&, uint64_t&);
    extern void (*unsignedModulo_i32)(int32_t, uint32_t, int32_t&, uint32_t&);
    extern void (*unsignedModulo_i16)(int16_t, uint16_t, int16_t&, uint16_t&);
    extern void (*unsignedModulo_i8) (int8_t, uint8_t, int8_t&, uint8_t&);
    extern void (*unsignedModulo_i)  (int, unsigned int, int&, unsigned int&);

    stCalendar y2038Calendar(time_t unixSeconds, int8_t* timeZone = nullptr, int8_t dst = -1);
    time_t y2038Set(stCalendar calendar);
}

#endif // Y2038CALENDAR_H
