// utf-8 (ü)

// MIT License
// Copyright © 2023 Olaf Simon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/**
 * @file   cTimeStd.cpp
 * @author Olaf Simon
 * @date   20.09.2023
 * @brief  Class LibCpp::cTime
 *
 * \addtogroup LibCpp_time
 * @{
 *
 * \class LibCpp::cTime
 *
 * Published under MIT License
 *
 * \section SECTION_TIME_DESIGN Designed features
 *
 * \subsection SECTION_TIME_MATH Direct mathematical operations with integrated  'timespan' functionality
 *
 * \code
 * cTime time = cTime::now();
 * cTime span = cTime::set(1, 2, 0, 0); // one day, two hours
 * cTime meetingTime = time + span;
 * \endcode
 *
 * \subsection SECTION_TIME_CAL Calendrical time representation with integrated time zone, thus integrated 'datetime' functionality
 *
 * \code
 * stCalendar calendar = meetingTime.calendar();
 * int day = calendar.day;
 * int zone = calendar.timeZone.hours;
 * \endcode
 *
 * \subsection SECTION_TIME_STRING Biunique string operations for printing and parsing (including time zone parsing and language support)
 *
 * \code
 * string timeText = meetingTime.toString();
 * cTime time = cTime::set("2020-03-15 15:30:10 CET")
 * \endcode
 *
 * \section SECTION_TIME_INSTALLATION Installation
 *
 * Add the following foulders and included files provided within the 'src' foulder to your C++ project.\n
 *
 * \code
 *  #include "LibCpp/Time/cTime.h"
 *  #include "LibOb/CppCommon/Time/LibOb_strptime.h"
 *  #include "LibOb/CppCommon/Language/Language.h"
 * \endcode
 *
 * LibCpp/Time/cTime.cpp\n
 * LibOb/Common/Time/LibOb_strptime.c\n
 * LibOb/Common/Language/Language.c and Language.h\n
 * \n
 * A sample main.cpp to build a demonstration project is also supplied:\n
 * \n
 *
 * \section SECTION_TIME_DESCRIPTION Description and examples for the 'cTime' class
 *
 * \subsection SECTION_TIME_MOTIVATIOM Motivation of class development
 *
 * Time operations are a standard task for programmers but even after over 50 years of electronic data processing,
 * the provided functions of programming languages like C/C++ are still unhandily.\n
 * For example the 'struct tm' used for calendrical time representation lacks an entry for the time zone
 * and the string to time conversion function 'strptime' lacks time zone parsing on Linux and is
 * even not existing on Windows.\n
 * This situaton was incredibly for me, especially because I (and other programmers) even couldn't find satisfying solutions on open source
 * publications.\n
 * <a href="https://stackoverflow.com/questions/1650715/is-there-a-standard-date-time-class-in-c">Time class c++ discussion on Stack Overflow</a>\n
 * The lack of 'strptime' on Windows is discussed on\n
 * <a href="https://stackoverflow.com/questions/321849/strptime-equivalent-on-windows">'strptime' for Windows discussion on Stack Overflow</a>\n
 * and a (by far best but still not optimal) solution including parsable time zones is published by P.J. Miller on github.\n
 * <a href="https://github.com/p-j-miller/date-time">'strptime' for Windows by J.P. Miller</a>\n
 * Alltogether this motivated me to develop and publish the 'cTime' class including a portable 'strptime' implementation.
 *
 *
 * \subsection SECTION_TIME_USAGE Usage of the 'cTime' class
 *
 * This code provides the class 'cTime' within the namespace 'LibCpp'. This class simplifies the
 * usage of date and time compared to the functions defined in 'time.h' and is based on those
 * functions, only.
 *
 * Each cTime instance stores the time as unix time, which are the seconds after 1.1.1970 00:00:00.
 * This integer value is the only member variable of the object.\n
 * Basically you have access to the value itself or to a calendar representation. The term 'calendar'
 * within this documentation always means calendar and time information in the human understandable form of
 * year, hour, minute and so on.
 *
 * \code
 * #include "cTime.h"
 *
 * using namespace LibCpp;
 *
 * int main()
 * {
 *     cTime  timeNow = cTime::now();
 *     time_t value = timeNow.time();          // Unix time representation
 *     stCalendar cal = timeNow.calendar();    // Calendar time representation
 *     printf("Unix time is: %d\n", (int)value);
 *     printf("Calendar  is: %s\n", cTime::toString(cal).c_str());
 *
 *     // Unix time is: 1695223058
 *     // Calendar  is: 2023-09-20#17:17:38#DST#+01:00
 * }
 * \endcode
 * The calendar is denoted in the 'Geographic Zone Calendar (\anchor GZC GZC) string'.\n
 * The term 'DST' means daylight saving time (dst). Possibly the term 'STD' for standard time appears.
 * In both cases '+01:00' stands for the geographic time zone the calendar data is given for.
 * It is important to distinguish between the geographic time zone and the relative deviation
 * of the given time to the Coordinated Universal Time (UTC) (same as Greenwich Mean Time (GMT)).\n
 * It is a deliberate decision not to use the ISO8601 time format for the calendar string. The
 * reason is the lack of ability to express the dst which is of importance for a meaningful understanding
 * of the date and time imformation. (E.g. birth certificates carry this information (at least if it is
 * necessary and C++ decided the same within the 'struct tm' within 'time.h'.) The most
 * obvious reason is the ambigious time during the hour of turning back the clock at the change from dst
 * to standard time. To distinguish between those two hours the dst information is necessary.\n
 * To receive the relative deviation (or UTC time offset), which is indicated by the dst value of -1
 * within the stCalendar struct, you can call:
 * \code
 *     cal = timeNow.calendar(cTime::asUTC);
 *     printf("Calendar  is: %s\n", cTime::toString(cal).c_str());
 *     // Calendar  is: 2023-09-20#17:17:38#UTC#+02:00
 * \endcode
 * It is also possible to "translate" date and time to another UTC relative time. (It is not possible
 * to translate to other geographic time zones as this would require dst-infomation to correctly
 * show the time. This is due to dst-information not being dependent on time zones but on specific countries.)
 * For example to UTC calendar.
 * \code
 *     cal = timeNow.calendar(cTime::UTC);
 *     printf("Calendar  is: %s\n", cTime::toString(cal).c_str());
 *     // Calendar  is: 2023-09-20#15:17:38#UTC#+00:00
 *     int8_t utcOffset = 5;
 *     cal = timeNow.calendar(&utcOffset);
 *     printf("Calendar  is: %s\n", cTime::toString(cal).c_str());
 *     // Calendar  is: 2023-09-20#20:17:38#UTC#+05:00
 * \endcode
 * It is also possible to calculate with cTime instances. First, there
 * are quite a few static cTime::set() methods to create a cTime instance
 * from different input values. Please refer to \ref cTime.h for complete
 * information. We will set christmas day and continue to calculate the
 * time duration we will have to wait till christmas day. As it doesn't make
 * sence to interprete a time difference as a calendar date, we can
 * represent a cTime instance as a time duration stored in a stDuration
 * struct containing days, hours, minutes and so on. Using years or months
 * doen't make sence in a scientific calculation as these values have
 * not constant durations. Take notice of how to access the year of our
 * current time 'timeNow'.
 * \code
 *     cTime timeChristmas = cTime::set(timeNow.calendar().year, 12, 24, 18, 0, 0);
 *     cTime timeWait = timeChristmas - timeNow;
 *     stDuration duration = timeWait.duration();
 *     printf("The wait time is: %s\n", cTime::toString(duration).c_str());
 *     // The wait time is: D95#00:42:22
 * \endcode
 * The 'D' indicates a duration, the time to wait is 95 days, 42 minutes and 22 seconds.\n
 * There are two more static methods to point out concerning the time zone and UTC time offset
 * calculation. Further more string conversion options.
 * \code
 *     printf("local time zone is: %+03d:00\n", (int)cTime::localTimeZone());
 *     printf("UTC deviation is  : %+03d:00\n", (int)cTime::UTCdeviation(timeNow.calendar()));
 *     cTime  time = cTime::set("2023-09-20#17:17:38#DST#+01:00");
 *     printf("Calendar is       : %s\n", time.toString().c_str());
 *     printf("Weekday is        : %s\n", weekdays[time.calendar().dayInWeek]);
 *     printf("Wait time is      : %s\n", timeWait.toDurationString().c_str());
 *     // local time zone is: +01:00
 *     // UTC deviation is  : +02:00
 *     // Calendar is       : 2023-09-20#20:17:38#DST#+01:00
 *     // Weekday is        : Wednesday
 *     // Wait time is      : D95#00:42:22
 * \endcode
 * For further available methods refer to \ref cTime.h.\n
 * \n
**/

//#if defined(LibCpp_Y2038) || defined(LibCpp_Y2038TEST)
//    #include "CalendarToTime.h"
//#else
//    #include "cTime.h"
//#endif

#include <time.h>
#include "cTime.h"

#define LibCpp_SECONDSPERHOUR 3600  ///< Konstante
#define LibCpp_SECONDSPERMINUTE 60  ///< Konstante

//! @cond Doxygen_Suppress
#define __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
//! @endcond

#include <cstring>

using namespace LibCpp;
using namespace std;

int8_t LibCpp::int8Zero = 0;        ///< Value of zero to let cTime::UTC point to.
int8_t LibCpp::int8_0x80 = 0x80;    ///< Value of 0x80 to let cTime::UTCdeviation point to.
const stCalendar LibCpp::stCalendar_Ini = {0, 0, 0, 0, 0, 0, 0, {0, 0}, 0, 0, 0, 0, 0, 0, 0}; ///< Initializer for stCalendar variables setting all entries to zero
const stCalendar LibCpp::stCalendar_IniUnix = {1970, 1, 1, 0, 0, 0, 0, {0, 0}, 0, 0, 0, 0, 0, 0, 0}; ///< Initializer for stCalendar variables which will be converted to unix time 0
const stCalendar LibCpp::stCalendar_Invalid = {INT32_INVALID, UINT8_INVALID, UINT8_INVALID, UINT8_INVALID, UINT8_INVALID, UINT8_INVALID, INT8_INVALID, {INT8_INVALID, 0}, 0, UINT16_INVALID, UINT8_INVALID, UINT8_INVALID, UINT8_INVALID, INT8_INVALID, INT16_INVALID}; ///< stCalendar_Invalid
const stDuration LibCpp::stDuration_Ini = {0, 0, 0, 0, 1};          ///< Initializer for stDuration
const stDuration LibCpp::stDuration_Invalid = {UINT64_INVALID, UINT64_INVALID, UINT64_INVALID, UINT64_INVALID, 1};   ///< Initializer for an invalid stDuration

/**
 * @brief Constructor
 */
cTime::cTime()
{
    _time = 0;
}

/**
 * @brief Deliveres a cTime instance holding the current time.
 * @return Created instance
 */
cTime cTime::now()
{
    cTime result;
    result._time = ::time(nullptr);
    return result;
}

/**
 * @brief Deliveres a cTime instance initialized with unixTime.
 * @param unixTime
 * @return Created instance
 */
cTime cTime::set(time_t unixTime)
{
    cTime t;
    t._time = unixTime;
    return t;
}

/**
 * @brief Deliveres a cTime instance initialized with the calendar data input.
 * The calendar data is to be provided as /ref stCalendar struct.\n
 * In case the time zone is indicated as invalid, the time zone of the local clock settings is used.
 * @param calendar
 * @return Created instance
 */
cTime cTime::set(stCalendar calendar)
{
    if (calendar.year == INT32_INVALID) calendar.year = 1970;
    if (calendar.month == UINT8_INVALID) calendar.month = 1;
    if (calendar.day == UINT8_INVALID) calendar.day = 1;
    if (calendar.hour == UINT8_INVALID) calendar.hour = 0;
    if (calendar.minute == UINT8_INVALID) calendar.minute = 0;
    if (calendar.second == UINT8_INVALID) calendar.second = 0;
    if (calendar.dst == INT8_INVALID) calendar.dst = 0;
    if (calendar.timeZone.hours == INT8_INVALID)
        calendar.timeZone = localTimeZone();

    struct tm tmCalendar = tm_Ini;
    tmCalendar.tm_year  = calendar.year - 1900;
    tmCalendar.tm_mon   = calendar.month - 1;
    tmCalendar.tm_mday  = calendar.day;
    tmCalendar.tm_hour  = calendar.hour;
    tmCalendar.tm_min   = calendar.minute;
    tmCalendar.tm_sec   = calendar.second;
    tmCalendar.tm_isdst = calendar.dst;
    tmCalendar.tm_isdst = -1;

    time_t timeValue = mktime(&tmCalendar);
    stTimeZone localZone = localTimeZone();
    if (timeValue==-1)
    {
        tmCalendar.tm_hour += localZone.hours;
        timeValue = mktime(&tmCalendar);
        timeValue -= 3600 * localZone.hours;
    }
    // subtract additional hour
    int dst = 0;
    if (tmCalendar.tm_isdst == 1) dst=1;
    if (tmCalendar.tm_isdst < 0) dst=0;
    timeValue += (-localZone.hours + dst + calendar.timeZone.hours) * LibCpp_SECONDSPERHOUR + calendar.timeZone.minutes * LibCpp_SECONDSPERMINUTE;
    return set(timeValue);
}

/**
 * @brief Deliveres a cTime instance initialized with the given time duration information.
 * The duration data is to be provided as /ref stDuration struct.
 * @param duration
 * @return Created instance
 */
cTime cTime::set(stDuration duration)
{
    time_t value = duration.days * 86400 + duration.hours * 3600 + duration.minutes * 60 + duration.seconds;
    if (duration.sign < 0) value = -value;
    return cTime::set(value);
}

/**
 * @brief Deliveres a cTime instance representing the given calendar data according to the local clock configuration.
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param minute
 * @param second
 * @return Created instance
 */
cTime cTime::set(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    stCalendar calendar = setCalendar(year, month, day, hour, minute, second);
    return cTime::set(calendar);
}

/**
 * @brief ///< Deliveres a cTime instance representing the given duration data.
 * @param days
 * @param hours
 * @param minutes
 * @param seconds
 * @param sign
 * @return Created instance
 */
cTime cTime::set(uint64_t days, uint64_t hours, uint64_t minutes, uint64_t seconds, int8_t sign)
{
    stDuration duration = stDuration_Ini;
    duration.days = days;
    duration.hours = hours;
    duration.minutes = minutes;
    duration.seconds = seconds;
    duration.sign = sign;
    return set(duration);
}

/**
 * @brief Deliveres a cTime instance initialized by a string representing either a calendar or a duration string.
 * @param dateString
 * @param format see strptime()
 * @return Created instance
 */
cTime cTime::set(std::string dateString, std::string format)
{
    if (dateString[0] == 'D')
    {
        stDuration duration = fromDurationString(dateString);
        return set(duration);
    }
    else
    {
        stCalendar calendar = fromString(dateString, format);
        if (memcmp(&calendar, &stCalendar_Invalid, sizeof(stCalendar))==0)
        {   // 'dateString' could not be interpreted as a calendar information and thus must be a duration string (withoud leading 'D')
            stDuration duration = fromDurationString(dateString);
            return set(duration);
        }
        return set(calendar);
    }
}

/**
 * @brief Returns the unix time stamp (seconds till 1.1.1970 00:00:00 GMT).
 * @return unix time
 */
time_t cTime::time()
{
    return _time;
}

/**
 * @brief Returns the calendar data representation of the instance.
 * Returns the memorized unix time as calendar data based on the local system clock configuration.
 * In case 'pRequestedTimeZone' is set and points to a int8_t variable containing the requested
 * UTC time deviation, the corresponding date and time is returned.\n
 * Use 'cTime::UTC' as parameter to receive the calendar data valid at UTC deviation zero.\n
 * Use 'cTime::asUTC' to keep the local date and time string but using UTC time deviation
 * instead of geographic time zone. This is useful for creating ISO8601 conform strings.
 * @param pRequestedTimeZone Pointer to a variable containing the desired UTC time deviation.
 * @return calendar struct
 */
stCalendar cTime::calendar(int8_t* pRequestedTimeZone)
{
    struct tm lt = tm_Ini;
    int8_t zone = localTimeZone().hours;
    //localtime_s(&lt, &_time);
    lt = *localtime(&_time);

    stCalendar calendar = stCalendar_Ini;

    if (pRequestedTimeZone)
    {
        int8_t relZone = zone;
        if (lt.tm_isdst>0)
            relZone += 1;

        int8_t relDestZone;
        if (*pRequestedTimeZone == INT8_INVALID)
        {
            relDestZone = zone;
            if (lt.tm_isdst > 0) relDestZone++;
        }
        else
            relDestZone = *pRequestedTimeZone;
        lt.tm_hour += relDestZone - relZone;
        mktime(&lt);
        lt.tm_isdst = -1;
        calendar.timeZone.hours = relDestZone;
    }
    else
        calendar.timeZone.hours = zone;

    calendar.second    = lt.tm_sec;         // seconds after the minute	0-60*
    calendar.minute    = lt.tm_min;         // minutes after the hour	0-59
    calendar.hour      = lt.tm_hour;        // hours since midnight	0-23
    calendar.day       = lt.tm_mday;        // day of the month	1-31
    calendar.month     = lt.tm_mon	+ 1;    // months since January	0-11
    calendar.year      = lt.tm_year	+ 1900; // years since 1900
    calendar.dst       = lt.tm_isdst;       // Daylight Saving Time flag
    calendar.dayInWeek = lt.tm_wday;
    if (!calendar.dayInWeek) calendar.dayInWeek = 7;
    calendar.dayInYear = lt.tm_yday + 1;

    return calendar;
}

/**
 * @brief Returns the internal unix time value as duration information.
 * @return Struct \ref _stDuration
 */
stDuration cTime::duration()
{
    stDuration duration;
    time_t value = _time;
    duration.sign = 1;
    if (value<0)
    {
        duration.sign = -1;
        value = -value;
    }
    duration.days = value / 86400;
    value = value % 86400;
    duration.hours = value / 3600;
    value = value % 3600;
    duration.minutes = value / 60;
    duration.seconds = value % 60;
    return duration;
}

/**
 * @brief Deliveres a \ref _stCalendar struct representing the given calendar data using the local geographic time zone
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param minute
 * @param second
 * @return Created calendar struct
 */
stCalendar cTime::setCalendar(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    stCalendar calendar = stCalendar_Ini;

    struct tm tmCalendar = tm_Ini;
    tmCalendar.tm_year  = year - 1900;
    tmCalendar.tm_mon   = month - 1;
    tmCalendar.tm_mday  = day;
    tmCalendar.tm_hour  = hour;
    tmCalendar.tm_min   = minute;
    tmCalendar.tm_sec   = second;
    tmCalendar.tm_isdst = -1;
    mktime(&tmCalendar);    // generates tm_isdst
    stTimeZone localZone = localTimeZone();
    calendar.dst      = tmCalendar.tm_isdst;
    calendar.timeZone = localZone;
    calendar.year     = year;
    calendar.month    = month;
    calendar.day      = day;
    calendar.hour     = hour;
    calendar.minute   = minute;
    calendar.second   = second;
    calendar.dayInWeek = tmCalendar.tm_wday;
    if (calendar.dayInWeek == 0) calendar.dayInWeek = 7;
    calendar.dayInYear = tmCalendar.tm_yday + 1;
    return calendar;
}

/**
 * @brief Returns the local time zone and dst information (if pointer is set) according to system clock settings.
 * In case dst is unknown (dst=-1) the time zone is the UTC time offset.
 * @param pDst Pointer to dst value [output]
 * @return Geographic time zone.
 */
stTimeZone cTime::localTimeZone(int8_t* pDst)
{
    return LibOb_localTimeZone(pDst);
}

/**
 * @brief Calculates the relative deviation from UTC (GMT) time.
 * @param zone
 * @param dst
 * @return
 */
stTimeZone cTime::UTCdeviation(stTimeZone zone, int8_t dst)
{
    stTimeZone result = zone;
    if (dst > 0 ) result.hours++;
    return result;
}

/**
 * @brief Delivers a calendar struct from a \ref GZC formatted string.
 * In case the 'dateString' is not a meaningful calendar date string stCalendar_Invalid is returned.
 * A meaningful calendar date string must contain either a year or a month (but should have both).
 * @param dateString Input date string.
 * @param format Format string as defined in strptime().
 * @return
 */
stCalendar cTime::fromString(std::string dateString, std::string format)
{
    struct tm tmCal = tm_Invalid;
    stTimeZone zone = stTimeZone_Invalid;

    if (dateString[0] == 'D') return stCalendar_Invalid;
    LibOb_strptime(dateString.c_str(), format.c_str(), &tmCal, &zone);
    if (tmCal.tm_year == INT_INVALID && tmCal.tm_mon == INT_INVALID)
    {   // 'dateString' cannot be meaningful interpretated as a calendar information and must be interpretated as a duration string
        return stCalendar_Invalid;
    }
    return toCalendar(tmCal, &zone);
}

/**
 * @brief Delivers a duration struct from a \ref GZC formatted string
 * A sample string is: D95#00:42:22.\n
 * In case a time string like "08:00" is given, which cannot be meaningful interpretated as a calendar date,
 * it will be also interpretated as a duration. In this case as 8 hours. A meaningful calendar date string
 * must contain either a year or a month (but should have both).
 * @param durationString
 * @return
 */
stDuration cTime::fromDurationString(std::string durationString, std::string format)
{
    stDuration duration = stDuration_Ini;
    if (durationString[0] != 'D' || !format.empty())
    {
        struct tm tmDur = tm_Invalid;
        stTimeZone zone = stTimeZone_Invalid;
        LibOb_strptime(durationString.c_str(), format.c_str(), &tmDur, &zone);
        if (tmDur.tm_year != INT_INVALID || tmDur.tm_mon != INT_INVALID)
        {   // 'durationString' is a meaningful calendar string and cannot be interpretated as duration string
            duration = stDuration_Invalid;
        }
        else
        {
            duration = toDuration(tmDur);
        }
    }
    else
    {
        int days = 0;
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        //sscanf_s(durationString.c_str(), "D%d#%d:%d:%d", &days, &hours, &minutes, &seconds);
        sscanf(durationString.c_str(), "D%d#%d:%d:%d", &days, &hours, &minutes, &seconds);
        duration.sign = 1;
        if (days < 0)
        {
            duration.sign = (int8_t)-1;
            days = -days;
        }
        duration.days = (uint64_t)days;
        duration.hours = (uint64_t)hours;
        duration.minutes = (uint64_t)minutes;
        duration.seconds = (uint64_t)seconds;
    }
    return duration;
}

/**
 * @brief Returns a string interpretation of the 'calendar' method result
 * See /ref calendar .
 * @param format see strptime()
 * @param pLanguage
 * @param pRequestedTimeZone
 * @return
 */
std::string cTime::toString(std::string format, enLanguage* pLanguage, int8_t* pRequestedTimeZone)
{
    stCalendar cal = calendar(pRequestedTimeZone);
    return cTime::toString(cal, format, pLanguage);
}

/**
 * @brief Returns a string representing a duration format
 * @return
 */
std::string cTime::toDurationString(std::string format)
{
    char buffer[64];
    struct tm t = fromDuration(duration());
    if (format.empty()) format = "D%d#%H:%M:%S";
    LibOb_strftime(buffer, 64, format.c_str(), &t, nullptr, nullptr);
    string result(buffer);
    return result;
}

/**
 * @brief Generates a \ref GZC formatted string from a calendar struct
 * @param calendar
 * @param format see strptime()
 * @param pLanguage
 * @return
 */
std::string cTime::toString(stCalendar calendar, std::string format, enLanguage* pLanguage)
{
    char buffer[64];
    stTimeZone zone;
    struct tm t = cTime::fromCalendar(calendar, &zone);
    if (format=="") format = "%Y-%m-%d#%H:%M:%S#%U#%z";
    LibOb_strftime(buffer, 64, format.c_str(), &t, &zone, pLanguage);
    string result(buffer);
    return result;
}

/**
 * @brief Generates a \ref GZC formatted string from a duration struct
 * @param duration
 * @return
 */
std::string cTime::toString(stDuration duration, std::string format)
{
    if (format.empty())
    {
        string str;
        char buffer[32];
        int days = (int)duration.days;
        if (duration.sign < 0) days = -days;
        //sprintf_s(buffer, 32, "D%d#%02d:%02d:%02d", days, (int)duration.hours, (int)duration.minutes, (int)duration.seconds);
        sprintf(buffer, "D%d#%02d:%02d:%02d", days, (int)duration.hours, (int)duration.minutes, (int)duration.seconds);
        string result(buffer);
        return result;
    }
    else
    {
        cTime dur = cTime::set(duration);
        return dur.toDurationString(format);
    }
}

/**
 * @brief Converts a stCalendar to a struct tm
 * If a pointer to a time zone is given, the time zone is also extracted.
 * @param calendar
 * @param pTimeZone
 * @return
 */
struct tm cTime::fromCalendar(stCalendar calendar, stTimeZone* pTimeZone)
{
    struct tm tmCalendar = tm_Invalid;
    if (calendar.year != INT32_INVALID)
        tmCalendar.tm_year  = calendar.year - 1900;
    if (calendar.month != UINT8_INVALID)
        tmCalendar.tm_mon   = calendar.month - 1;
    if (calendar.day != UINT8_INVALID)
        tmCalendar.tm_mday  = calendar.day;
    if (calendar.hour != UINT8_INVALID)
        tmCalendar.tm_hour  = calendar.hour;
    if (calendar.minute != UINT8_INVALID)
        tmCalendar.tm_min   = calendar.minute;
    if (calendar.second != UINT8_INVALID)
        tmCalendar.tm_sec   = calendar.second;
    if (calendar.dst != INT8_INVALID)
        tmCalendar.tm_isdst = calendar.dst;
    if (calendar.dayInWeek != UINT8_INVALID)
    {
        tmCalendar.tm_wday  = calendar.dayInWeek;
        if (tmCalendar.tm_wday == 7) tmCalendar.tm_wday = 0;
    }
    if (calendar.dayInYear != UINT16_INVALID)
        tmCalendar.tm_yday  = calendar.dayInYear - 1;
    if (pTimeZone)
    {
        if (calendar.year != INT8_INVALID)
            *pTimeZone = calendar.timeZone;
    }
    return tmCalendar;
}

/**
 * @brief Converts a struct tm to a BibCpp::stCalendar .
 * If no pointer to time zone is specified (standard), dst is set according to the local system clock settings.
 * Entities of stCalendar not available in struct tm are left to initial values.
 * @param tmCalendar
 * @param pTimeZone
 * @return
 */
stCalendar cTime::toCalendar(struct tm tmCalendar, const stTimeZone* pTimeZone)
{
    stCalendar calendar = stCalendar_Invalid;
    if (tmCalendar.tm_year != INT_INVALID)
        calendar.year = tmCalendar.tm_year + 1900;
    if (tmCalendar.tm_mon != INT_INVALID)
        calendar.month = tmCalendar.tm_mon + 1;
    if (tmCalendar.tm_mday != INT_INVALID)
        calendar.day = tmCalendar.tm_mday;
    if (tmCalendar.tm_hour != INT_INVALID)
        calendar.hour = tmCalendar.tm_hour;
    if (tmCalendar.tm_min != INT_INVALID)
        calendar.minute = tmCalendar.tm_min;
    if (tmCalendar.tm_sec != INT_INVALID)
        calendar.second = tmCalendar.tm_sec;
    if (tmCalendar.tm_isdst != INT_INVALID)
        calendar.dst = tmCalendar.tm_isdst;
    if (tmCalendar.tm_wday != INT_INVALID)
    {
        calendar.dayInWeek = tmCalendar.tm_wday;
        if (!calendar.dayInWeek) calendar.dayInWeek = 7;
    }
    if (tmCalendar.tm_year != INT_INVALID)
        calendar.dayInYear = tmCalendar.tm_yday + 1;
    // In case neither a month nor a year is given, time zone information does
    // not make sense and the result can only be treated as a time duration
    if ((tmCalendar.tm_year != INT_INVALID) || (tmCalendar.tm_mon != INT_INVALID))
    {
        if (pTimeZone)
        {
            if (pTimeZone->hours != INT8_INVALID)
                calendar.timeZone = *pTimeZone;
            else
                calendar.timeZone = cTime::localTimeZone(&calendar.dst);
        }
        else
        {
            calendar.timeZone = cTime::localTimeZone(&calendar.dst);
        }
    }
    return calendar;
}

/**
 * @brief Converts a struct duration to struct tm and time zone
 * @param duration
 * @return
 */
struct tm cTime::fromDuration(stDuration duration)
{
    struct tm tmDuration = tm_Invalid;
    if (duration.days != UINT8_INVALID)
        tmDuration.tm_mday  = duration.days;
    if (duration.hours != UINT8_INVALID)
        tmDuration.tm_hour  = duration.hours;
    if (duration.minutes != UINT8_INVALID)
        tmDuration.tm_min   = duration.minutes;
    if (duration.seconds != UINT8_INVALID)
        tmDuration.tm_sec   = duration.seconds;
    return tmDuration;
}

/**
 * @brief Converts struct tm to a struct duration
 * @param tmDuration
 * @return
 */
stDuration cTime::toDuration(struct tm tmDuration)
{
    stDuration duration = stDuration_Ini;
    if (tmDuration.tm_mday != INT_INVALID)
        duration.days = tmDuration.tm_mday;
    if (tmDuration.tm_hour != INT_INVALID)
    {
        if (tmDuration.tm_hour>=24)
        {
            duration.days += tmDuration.tm_hour / 24;
            tmDuration.tm_hour = tmDuration.tm_hour % 24;
        }
        duration.hours = tmDuration.tm_hour;
    }
    if (tmDuration.tm_min != INT_INVALID)
        duration.minutes = tmDuration.tm_min;
    if (tmDuration.tm_sec != INT_INVALID)
        duration.seconds = tmDuration.tm_sec;
    return duration;
}

/**
 * @brief operator <<
 * @param out Output stream.
 * @param t cTime instance to be streamed as text representation.
 * @return
 */
std::ostream & operator << (std::ostream &out, cTime &t)
{
    out << t.toString();
    return out;
}

/**
 * @brief operator >>
 * @param in Input stream.
 * @param t
 * @return
 */
std::istream & operator >> (std::istream &in, cTime &t)
{
    string s;
    in >> s;
    t = cTime::set(s);
    return in;
}

/** @} */
