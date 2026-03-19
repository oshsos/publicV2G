#include "cTimer.h"

using namespace std::chrono;
using namespace LibCpp;

/**
 * @brief Returns the current time in ns since epoche.
 * @return
 */
uint64_t cTimer::now()
{
    if (nano)
    {
        time_point<system_clock, nanoseconds> startTime = system_clock::now();
        nanoseconds relTime = startTime.time_since_epoch();
        return relTime.count();
    }
    else
    {
        time_point<system_clock, nanoseconds> startTime = system_clock::now();
        nanoseconds relTime = startTime.time_since_epoch();
        return relTime.count()/(long long)1e6;
    }
}

/**
 * @brief Time difference a-b
 * Returns the time difference a-b, assuming a>b.
 * @param a
 * @param b
 * @return a-b
 */
uint64_t cTimer::diffp(uint64_t a, uint64_t b)
{
    int64_t diff = (int64_t)a - (int64_t)b;
    return (uint64_t)diff;
}

/**
 * @brief Constructor
 * The Constructor calls the 'start' method.
 * @param duration
 */
cTimer::cTimer(uint64_t duration, bool start, bool nanoseconds)
{
    nano = nanoseconds;
    if (start)
        this->start(duration);
    else
        this->duration = duration;
    startTime = cTimer::now();
}

/**
 * @brief Sets the start time and clears the reset counter
 * @param duration An internal memorized duration the timer should measure
 */
void cTimer::start(uint64_t duration)
{
    startCount = 0;
    if (duration) this->duration = duration;
    restart();
}

/**
 * @brief Sets a new start time and inreases the reset counter
 */
void cTimer::restart()
{
    startTime = cTimer::now();
    if (duration)
        startCount++;
}

/**
 * @brief Forces the parameterless elapsed method to return -1 to indicate the timer being inactive (no time out has occurred)
 * Ends the occurrence of a timeout.
 */
uint64_t cTimer::stop()
{
    startCount = 0;
    return get();
}

/**
 * @brief Returns the time elepsed since the last timer restart.
 * @return
 */
uint64_t cTimer::get()
{
    uint64_t nowTime = now();
    return diffp(nowTime, startTime);
}

/**
 * @brief Checks whether the duration has been elapsed since startTime
 * Returns the number of reset counts or zero if duration has not been passed yet.
 * In case duration is unset, the internal duration set by 'start' is used.
 */
int cTimer::elapsed(uint64_t duration)
{
    uint64_t actElapsedTime = get();
    if (duration)
    {
        if (actElapsedTime > duration)
            return 1;
        return 0;
    }
    if (this->duration == 0 || startCount == 0)
        return -1;
    if (actElapsedTime > this->duration)
        return startCount;
    return 0;
}

/**
 * @brief Returns a time value of current time plus duration.
 * @param duration
 * @return
 */
uint64_t cTimer::getAhead(uint64_t duration)
{
    return get() + duration;
}

/**
 * @brief Checks, whether the current time is later as or equal to the reference time.
 * @param refTime Reference time
 * @return
 */
bool cTimer::passed(uint64_t refTime)
{
    return get() >= refTime;
}

/**
 * @brief Returns the duration elapsed since the reference time based on the difference to the current time.
 * @param refTime Reference time
 * @return
 */
uint64_t cTimer::elapsedSince(uint64_t refTime)
{
    return diffp(get(), refTime);
}
