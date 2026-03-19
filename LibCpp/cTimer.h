#ifndef CTIMER_H
#define CTIMER_H

#include <chrono>

namespace LibCpp
{

/** @brief A timer in ns precision
 *  The timer is implemented in a passive manner, thus time is only measured on calling
 *  a method. Thus, no callbacks can be registered.
 */
class cTimer
{
    static constexpr uint64_t s =  (uint64_t)1e9;     ///< nanoseconds per second
    static constexpr uint64_t ms = (uint64_t)1e6;     ///< nanoseconds per millisecond
    static constexpr uint64_t us = (uint64_t)1e3;     ///< nanoseconds per microsecond

public:
    cTimer(uint64_t duration = 0, bool start = false, bool nanoseconds = false);          ///< Calls 'start'

    void start(uint64_t duration = 0);      ///< Sets the start time and clears the reset counter
    void restart();                         ///< Sets a new start time and inreases the reset counter
    uint64_t stop();                        ///< Forces the parameterless elapsed method to return -1 to indicate the timer being inactive (no time out has occurred). Returns get().
    uint64_t get();                         ///< Returns the time elepsed since the last timer restart.
    int  elapsed(uint64_t duration = 0);    ///< Checks whether the duration has been elapsed since startTime and returns the number of reset counts increased by one or zero if still running. In case duration is unset, the internal duration set by 'start' is used.

    uint64_t getAhead(uint64_t duration);   ///< Returns a time value of current time plus duration.
    bool passed(uint64_t refTime);          ///< Checks, whether the current time is later as or equal to the reference time.
    uint64_t elapsedSince(uint64_t refTime);    ///< Returns the duration elapsed since the reference time based on the difference to the current time.

    uint64_t now();                  ///< Returns the current time in ns since epoche
    static uint64_t diffp(uint64_t a, uint64_t b);  ///< Returns the time difference a-b, assuming a>b.

private:
    uint64_t startTime;
    uint64_t duration;
    int startCount;
    bool nano;

};

}

#endif
