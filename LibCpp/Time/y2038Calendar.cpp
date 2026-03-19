/**
 * search: c++ class time
 * https://en.cppreference.com/w/cpp/chrono
 * https://stackoverflow.com/questions/1650715/is-there-a-standard-date-time-class-in-c
 * https://www.boost.org/doc/libs/1_55_0/doc/html/date_time/date_time_io.html#date_time.io_tutorial
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0355r1.html
 * https://doc.qt.io/qt-5/qtime.html
 *
 * https://unixtime.org/
 * 2038 problem, LibCpp_EPOCHEYEAR
 * UTC(coordinated universal time) GMT Greenwich Mean Time
 * https://stackoverflow.com/questions/21593692/convert-unix-timestamp-to-date-without-system-libs
 * mktime fails for 1.1.1970 CET
 * gmtime provides data having dst set
 * trouble with time 'zone' https://stackoverflow.com/questions/34730939/iso-8601-with-daylight-savings
 * problem der Uhrumstellung -> dst is important
 * Confusion with switching utc times (lack of standard, utc becomes meaningless) CEST=utc+02:00 becomes nonreversible utc+2 = CEST or xx
 * 1988
 * Evaluating utc time is probably not reliable possible if no DST and geographic time zone information is present.
 * STD/DST/UTC is just a reference for the wall clock value, and is not (necessarily) correct with respect to local roules within the given
 * geographical time zone (and cannot because this definition is country dependent and may differ in the same geographical time zone).
 * UTC is used to transform ISO8601 into geographical format. c++ uses dst!
 *
 * Implementation is based on standard c++ functions defined in time.h
 *
 * Geographic Zone Calendar (GZC) string. Locally meaningful calendar and wall clock data being world wide convertible.
 * Calendar string: 2023-06-03#12:30:23#DST#+01:00
 * Duration string: D-324#02:34:10
 * wall clock ambigious, wall clock distict, wall clock UTC convertible
 *
 * @brief cTime::localTimeZone
 * @param relative = false
 * @return geographical or relative (if parameter 'relative' is set) time zone or LibCpp_INVALIDTIMEZONE
 *
 * This method returns the local time zone as geographical time zone. This means
 * the value will be the same independent of the daylight saving time. As the detection
 * of an active daylight saving (DST) time may fail this method may deliver LibCpp_INVALIDTIMEZONE as result
 * to indicate such an failure.\n
 * To guarantee a valid result set parameter 'relative' to 'true'. This will result in the number of
 * hours your current wall clock (or visible) time differs from UTC.\n
 * It is common practice to fall back to the relative time zone in case the absolute time zone cannnot
 * be retrieved, due to unachievable DST. In such a case the 'tm_isdst' in the calender 'struct tm' must be set
 * to minus one indicating this matter. In this case relative and geographical time zones are identical and the calender and time zone
 * information can be correctly calculated back to UTC time.
 * \code
 * struct tm calendar;
 * int8_t zone;
 * zone = cTimer::localTmeZone();
 * if (zone == LibCpp_INVALIDTIMEZONE)
 * {
 *     zone = cTimer::localTimeZone(true);
 *     calendar.tm_isdst = 0;
 * }
 * \endcode
 * To do so automatically you can pass a pointer to a 'struct tm' variable as second parameter.
 * If you do so, that variable will be set to the local calender (see method localCalender()), maybe
 * assuming standard time instead of summer time.
 * \code
 * struct tm calendar;
 * int8_t zone;
 * zone = cTimer::localTmeZone();
 * if (zone == LibCpp_INVALIDTIMEZONE)
 * {
 *     zone = cTimer::localTimeZone(true);
 *     calendar.tm_isdst = -1;
 * }
 * \endcode
 * The methods localCalendar() combined with localTimeZone() deliver a full data set to be able to
 * recalculate UTC time.
**/

//#define LibCpp_Y2038
//#define LibCpp_Y2038TEST

#ifdef LibCpp_CONFIG
    #include <LibCpp_CONFIG.h>
#endif

#ifndef LibCpp_Y2038            ///< Interpretates unix time with respect to a new epoche year 2030 on 32 bit time_t systems (and only on such systems).
    #define LibCpp_Y2038_ false
#else
    #define LibCpp_Y2038_ true
#endif

#ifndef LibCpp_Y2038TEST        ///< For test purplose limits time_t to 32 bit on 64 bit systems and uses the epoche year 2030.
    #define LibCpp_Y2038TEST_ false
#else
    #define LibCpp_Y2038TEST_ true
#endif

/** Calendar specific constants **/
#define LibCpp_SECONDSPERMINUTE  60
#define LibCpp_MINUTESPERHOUR    60
#define LibCpp_HOURSPERDAY       24
#define LibCpp_DAYSPERNORMALYEAR 365
#define LibCpp_LEAPDAY           1
#define LibCpp_DAYSPERWEEK       7
#define LibCpp_MONTHPERYEAR      12

#define LibCpp_DAYSOFJAN         31
#define LibCpp_DAYSOFFEB         28
#define LibCpp_DAYSOFMAR         31
#define LibCpp_DAYSOFAPR         30
#define LibCpp_DAYSOFMAY         31
#define LibCpp_DAYSOFJUN         30
#define LibCpp_DAYSOFJUL         31
#define LibCpp_DAYSOFAUG         31
#define LibCpp_DAYSOFSEP         30
#define LibCpp_DAYSOFOKT         31
#define LibCpp_DAYSOFNOV         30
#define LibCpp_DAYSOFDEC         31

/** UnixTime specific **/
#define LibCpp_EPOCHEYEAR 1970
#define LibCpp_NEWEPOCHEYEAR 2030
#define LibCpp_TMYEAR 1900
#define LibCpp_TIME_T_2001 ((int64_t) 978307200)
#define LibCpp_TIME_T_2030 ((int32_t)1893456000)
#define LibCpp_TIME_T_2030_2001 (LibCpp_TIME_T_2030 - LibCpp_TIME_T_2001)

/** Leap year roules dependent constant time segments. They all are designed to have the leap year roule dependent year at the end. The beginning year starts from 1.1.---- 00:00:00 til 31.12.(x*block-length) **/
#define LibCpp_DAYSPER4YEARS   ( 4 * LibCpp_DAYSPERNORMALYEAR + LibCpp_LEAPDAY) ///< The i-times 4 year block includes the years from (i-1)*4+1 to n*4 [one leap year every 4 years]
#define LibCpp_DAYSPER100YEARS (25 * LibCpp_DAYSPER4YEARS     - LibCpp_LEAPDAY) ///< The j-times 100 year block includes the years from (j-1)*100+1 to n*100 [no leap year every 100 years]
#define LibCpp_DAYSPER400YEARS ( 4 * LibCpp_DAYSPER100YEARS   + LibCpp_LEAPDAY) ///< The k-times 400 year block includes the years from (n-1)*400+1 to n*400 [remaining one missed leap year]

#define LibCpp_SECONDSOFDAY   (LibCpp_HOURSPERDAY * LibCpp_SECONDSPERHOUR)
#define LibCpp_SECONDSTILLFEB (LibCpp_DAYSOFJAN * LibCpp_SECONDSOFDAY)
#define LibCpp_SECONDSTILLMAR (LibCpp_DAYSOFFEB * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLFEB)
#define LibCpp_SECONDSTILLAPR (LibCpp_DAYSOFMAR * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLMAR)
#define LibCpp_SECONDSTILLMAY (LibCpp_DAYSOFAPR * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLAPR)
#define LibCpp_SECONDSTILLJUN (LibCpp_DAYSOFMAY * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLMAY)
#define LibCpp_SECONDSTILLJUL (LibCpp_DAYSOFJUN * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLJUN)
#define LibCpp_SECONDSTILLAUG (LibCpp_DAYSOFJUL * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLJUL)
#define LibCpp_SECONDSTILLSEP (LibCpp_DAYSOFAUG * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLAUG)
#define LibCpp_SECONDSTILLOKT (LibCpp_DAYSOFSEP * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLSEP)
#define LibCpp_SECONDSTILLNOV (LibCpp_DAYSOFOKT * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLOKT)
#define LibCpp_SECONDSTILLDEC (LibCpp_DAYSOFNOV * LibCpp_SECONDSOFDAY + LibCpp_SECONDSTILLNOV)

#include <time.h>
#include <stdint.h>

#include "y2038Calendar.h"

const uint64_t LibCpp_SECONDSPERHOUR        = (uint64_t)LibCpp_MINUTESPERHOUR    * LibCpp_SECONDSPERMINUTE;
const uint64_t LibCpp_SECONDSPERDAY         = (uint64_t)LibCpp_HOURSPERDAY       * LibCpp_SECONDSPERHOUR;
const uint64_t LibCpp_SECONDSPERWEEK        = (uint64_t)LibCpp_DAYSPERWEEK       * LibCpp_SECONDSPERDAY;
const uint64_t LibCpp_SECONDSPERNORMALYEAR  = (uint64_t)LibCpp_DAYSPERNORMALYEAR * LibCpp_SECONDSPERDAY;
const uint64_t LibCpp_SECONDSPER4YEARS      = LibCpp_DAYSPER4YEARS               * LibCpp_SECONDSPERDAY;
const uint64_t LibCpp_SECONDSPER100YEARS    = LibCpp_DAYSPER100YEARS             * LibCpp_SECONDSPERDAY;
const uint64_t LibCpp_SECONDSPER400YEARS    = LibCpp_DAYSPER400YEARS             * LibCpp_SECONDSPERDAY;

const uint64_t LibCpp_SECONDSTILLMONTH[2][12] = {{0, LibCpp_SECONDSTILLFEB, LibCpp_SECONDSTILLMAR, LibCpp_SECONDSTILLAPR, LibCpp_SECONDSTILLMAY, LibCpp_SECONDSTILLJUN, LibCpp_SECONDSTILLJUL, LibCpp_SECONDSTILLAUG, LibCpp_SECONDSTILLSEP, LibCpp_SECONDSTILLOKT, LibCpp_SECONDSTILLNOV, LibCpp_SECONDSTILLDEC},
                                                 {0, LibCpp_SECONDSTILLFEB, LibCpp_SECONDSTILLMAR + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLAPR + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLMAY + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLJUN + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLJUL + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLAUG + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLSEP + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLOKT + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLNOV + LibCpp_SECONDSOFDAY, LibCpp_SECONDSTILLDEC + LibCpp_SECONDSOFDAY}};

using namespace LibCpp;

void (*LibCpp::unsignedModulo_i64)(int64_t, uint64_t, int64_t&, uint64_t&) = &unsignedModulo<int64_t, uint64_t>;
void (*LibCpp::unsignedModulo_i32)(int32_t, uint32_t, int32_t&, uint32_t&) = &unsignedModulo<int32_t, uint32_t>;
void (*LibCpp::unsignedModulo_i16)(int16_t, uint16_t, int16_t&, uint16_t&) = &unsignedModulo<int16_t, uint16_t>;
void (*LibCpp::unsignedModulo_i8 )(int8_t, uint8_t, int8_t&, uint8_t&) = &unsignedModulo<int8_t, uint8_t>;
void (*LibCpp::unsignedModulo_i  )(int, unsigned int, int&, unsigned int&) = &unsignedModulo<int, unsigned int>;

/**
 * @brief Calculates clock and calendar data for 32bit unix times and solves the year 2038 overflow by using year 2030 as epoch.
 * Calculates time data like hour and calendar data like month.\n
 * This function is used if \#define LibCpp_Y2038 is set for compilation and running on a 32 bit unix time system.\n
 * Use LibCpp_Y2038TEST to simulate 32 bit behavior on 64 bit unix time systems.\n
 * Typical (indirect) usage is:
 * \code
 * cTime t = cTime::now();
 *
 * // Calendar data according to local time zone
 * stCalendar cal = t.calendar();
 *
 * // Calendar data according to UTC (GMT) time zone
 * stCalendar cal = t.calendar(cTime::UTC);
 *
 * // Calendar data according to a given (relative) time zone
 * int8_t zone = 2;
 * stCalendar cal = t.calendar(&zone);
 *
 * // Calendar data according to a given absolute time zone under the assumption of a daylight saving time.
 * int8_t zone = 2;
 * stCalendar cal = t.calendar(&zone, 1);
 * \endcode
 *
 * @param unixSeconds Possibly overflown unix time according to epoche year 1970.
 * @param timeZone    Null pointer for local time zone (standard). Pointer to value zero (cTime::UTC) for Greenwich mean time. Pointer to requested geographic time zone (standard time, independent of dst). If dst is unknown, reative time zone (UTC time zone).
 * @param dst         Daylight saving time active (1), inaktive (0) or undefined (-1). Must be -1 (standard) or 0 to receive UTC time.
 * @return            Human readable time and calendar dates
 */
stCalendar LibCpp::y2038Calendar(time_t unixSeconds, int8_t* timeZone, int8_t dst)
{
    stCalendar calendar = stCalendar_Ini;

    int32_t time32 = (int32_t)unixSeconds;      // Convert to 32 bit (needed for test purpose)
    time32 -= LibCpp_TIME_T_2030;               // Roll to epoche 2030
    int64_t time64 = (int64_t)time32;
    // Roll the time to be relative to the very begin of 2001, which is the begin of a 400 year block.
    // The years before zero seconds belong to the 5th block ending with the year 2000,
    // the values after and including zero seconds belong to the 6th block.
    time64 += LibCpp_TIME_T_2030_2001;   // Sets time64 to start with 2001

    // Shift the time to the required time zone
    int8_t localZone;
    time_t time = ::time(nullptr);
    struct tm tmCalendar;
    localZone = cTime::localTimeZone();
    localtime_s(&tmCalendar, &time);
    int8_t relZone = 0;
    calendar.dst = dst;
    if (!timeZone)
    {
        dst = tmCalendar.tm_isdst;
        if (dst>0)
            relZone = localZone + 1;
        else
            relZone = localZone;
        calendar.timeZone = localZone;
    }
    else
    {
        relZone = *timeZone;
        if (dst>0)
            relZone -= 1;
        calendar.timeZone = *timeZone;
    }
    time64 += (int64_t)relZone * LibCpp_SECONDSPERHOUR;

    // 400 year block index
    int64_t k;
    uint64_t block400;
    unsignedModulo_i64(time64, LibCpp_SECONDSPER400YEARS, k, block400);
    // 100 year block index
    int64_t j;
    uint64_t block100;
    unsignedModulo_i64(block400, LibCpp_SECONDSPER100YEARS, j, block100);
    // 4 year block index
    int64_t i;
    uint64_t block4;
    unsignedModulo_i64(block100, LibCpp_SECONDSPER4YEARS, i, block4);
    // single year
    int64_t h;
    uint64_t block1;
    unsignedModulo_i64(block4, LibCpp_SECONDSPERNORMALYEAR, h, block1);

    // Days from start of 2001 till start of current year
    int64_t daysBeginSince2001 = k * LibCpp_DAYSPER400YEARS + (int32_t)j * LibCpp_DAYSPER100YEARS + i * LibCpp_DAYSPER4YEARS + (int32_t)h * LibCpp_DAYSPERNORMALYEAR;
    k += 5; // block 0 starts at year 2001 = 400 * (k=5) + (h=1)
    h += 1; // see comment above at k += 5
    int32_t year = (int32_t)k * 400 + (int32_t)j * 100 + (int32_t)i * 4 + (int32_t)h;
    calendar.year = year;

    int il = LibCpp_isLeapYear(year);
    //if (il) dayssince2001++;
    int m;
    for (m=11; m>=0; m--)
        if (block1 >= LibCpp_SECONDSTILLMONTH[il][m])
            break;
    uint64_t blockM = block1 - LibCpp_SECONDSTILLMONTH[il][m];
    calendar.month = m+1;

    int64_t d;
    uint64_t blockD;
    unsignedModulo_i64(blockM, LibCpp_SECONDSPERDAY, d, blockD);
    calendar.day = d + 1;

    int64_t hour;
    uint64_t blockHr;
    unsignedModulo_i64(blockD, LibCpp_SECONDSPERHOUR, hour, blockHr);
    calendar.hour = hour;

    int64_t minute;
    uint64_t second;
    unsignedModulo_i64(blockHr, LibCpp_SECONDSPERMINUTE, minute, second);
    calendar.minute = minute;
    calendar.second = second;

    uint64_t block;
    int64_t dayInYear;
    unsignedModulo_i64(block1, LibCpp_SECONDSPERDAY, dayInYear, block);
    dayInYear++;
    calendar.dayInYear = dayInYear;

    // Year 2001 starts with Monday 1. Jan
    uint64_t blockW;
    int64_t dWeeks2001;  // number off full weeks since Start of 2001 before the current day
    unsignedModulo_i64(time64, LibCpp_SECONDSPERWEEK, dWeeks2001, blockW);
    int64_t yWeeks2001 = daysBeginSince2001 / 7;
    calendar.calendarWeek = (uint8_t)(dWeeks2001 - yWeeks2001 +1);

    uint64_t blockX;
    int64_t dw;
    unsignedModulo_i64(blockW, LibCpp_SECONDSPERDAY, dw, blockX);
    calendar.dayInWeek = dw+1;

    return calendar;
}

/**
 * @brief Calculates the 32 bit unix time 1970 year epoch, interpreting the calendar to the epoche 2030.
 * The calender data will be transformed to a year 2030 epoch, which is back transformed to the 1970 epoch possibly containing an overflow.\n
 * The usable calendar input range is roughly from 1970 till 2090.
 * @param calendar
 * @return 32 bit unix time (possibly including the year 2038 overflow)
 */
time_t LibCpp::y2038Set(stCalendar calendar)
{
    int64_t time64 = 0;
    int64_t y = calendar.year - 2001;
    int64_t k;
    uint64_t rem400;
    unsignedModulo_i64(y, 400, k, rem400);
    int64_t j;
    uint64_t rem100;
    unsignedModulo_i64(rem400, 100, j, rem100);
    // 4 year block index
    int64_t i;
    uint64_t h;
    //uint64_t rem4;
    unsignedModulo_i64(rem100, 4, i, h);
    time64 += k * LibCpp_SECONDSPER400YEARS;
    time64 += j * LibCpp_SECONDSPER100YEARS;
    time64 += i * LibCpp_SECONDSPER4YEARS;
    time64 += h * LibCpp_SECONDSPERNORMALYEAR;
    int il = LibCpp_isLeapYear(calendar.year);
    time64 += LibCpp_SECONDSTILLMONTH[il][calendar.month-1];
    time64 += (calendar.day - 1) * LibCpp_SECONDSPERDAY;
    time64 += calendar.hour * LibCpp_SECONDSPERHOUR;
    time64 += calendar.minute * LibCpp_SECONDSPERMINUTE;
    time64 += calendar.second;
    int8_t relZone = calendar.timeZone;
    if (calendar.dst > 0)
        relZone += 1;
    time64 -= relZone * LibCpp_SECONDSPERHOUR;
    time64 -= LibCpp_TIME_T_2030_2001;   // Sets time64 to start with 2030
    int32_t time32 = (int32_t)time64;
    time32 += LibCpp_TIME_T_2030;
    return (time_t)time32;
}

/**
 * @brief Calculates divisor and remainder out of a signed value and unsigned moduo
 * The result of a given input value and modulo value is stored in divisor and remainder.
 * The multible of 'devisor' is the next value being than value! This is not the case for
 * the standard modulo operation % in case of negative values.\n
 * The result fulfills the following equation:\n
 * value = divisor * modulo + remainder
 * @param value
 * @param modulo
 * @param divisor
 * @param remainder
 */
template <typename T, typename uT> void LibCpp::unsignedModulo(T value, uT modulo, T& divisor,  uT& remainder)
{
    T div, rem;
    if (modulo == 0)
    {
        divisor  = 1;
        remainder = 0;
    }
    else
    {
        div = value / (T)modulo;
        rem = value % (T)modulo;
        if (div < 0)
        {
            div -= 1;
            if (rem < 0)
                rem += modulo;
        }
        if (div == 0 && rem < 0)
        {
            rem += modulo;
            div -= 1;
        }
        remainder = (uT)rem;
        divisor = div;
    }
}

namespace LibCpp
{
extern const uint8_t DAYSOFMONTH[2][13];
}

const uint8_t LibCpp::DAYSOFMONTH[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                                            {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

void setTimeZone(stCalendar* pCalendar, uint8_t destZone)
{
    int il = LibCpp_isLeapYear(pCalendar->year);
    int8_t diffHours = destZone - pCalendar->timeZone;
    if (pCalendar->dst >0) diffHours++;
    if (diffHours>0)
    {
        int8_t hour = pCalendar->hour + diffHours;
        if (hour > 23)
        {
            pCalendar->hour = hour - 24;
            int8_t day = pCalendar->day + 1;
            int8_t month = pCalendar->month;
            if (day > DAYSOFMONTH[il][month])
            {
                day = 1;
                month = month + 1;
                if (month > 12)
                {
                    pCalendar->day = day;
                    pCalendar->month = 1;
                    pCalendar->year += 1;
                }
                else
                {
                    pCalendar->day = day;
                    pCalendar->month = month;
                }
            }
            else
                pCalendar->day = day;
        }
        else
           pCalendar->hour = hour;
        pCalendar->dst = 0;
    }
    else if (diffHours<0)
    {
        int8_t hour = pCalendar->hour + diffHours;
        if (hour < 0)
        {
            pCalendar->hour = hour + 24;
            int8_t day = pCalendar->day - 1;
            int8_t month = pCalendar->month;
            if (day < 0)
            {
                month = month - 1;
                if (month < 1)
                {
                    month = 12;
                    pCalendar->day = DAYSOFMONTH[il][month];
                    pCalendar->month = month;
                    pCalendar->year -= 1;
                }
                else
                {
                    pCalendar->day = DAYSOFMONTH[il][month];
                    pCalendar->month = month;
                }
            }
            else
                pCalendar->day = day;
        }
        else
           pCalendar->hour = hour;
        pCalendar->dst = 0;
    }
};
