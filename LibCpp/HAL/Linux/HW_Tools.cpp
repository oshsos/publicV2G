#include "../HW_Tools.h"

#include <string.h>
#include <unistd.h>

using namespace std;
using namespace LibCpp;

void LibCpp::BigEndian(int16_t* pValue)
{
    char* p = (char*)pValue;
    char x = pValue[0];
    p[0] = p[1];
    p[1] = x;
}

void LibCpp::BigEndian(int32_t* pValue)
{
    char* p = (char*)pValue;
    char x = p[0];
    p[0] = p[3];
    p[3] = x;
    x = p[1];
    p[1] = p[2];
    p[2] = x;
}

void LibCpp::BigEndian(int64_t* pValue)
{
    char* p = (char*)pValue;
    char x = p[0];
    p[0] = p[7];
    p[7] = x;
    x = p[1];
    p[1] = p[6];
    p[6] = x;
    x = p[2];
    p[2] = p[5];
    p[5] = x;
    x = p[3];
    p[3] = p[4];
    p[4] = x;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void LibCpp::LittleEndian(int16_t* pValue) {};
void LibCpp::LittleEndian(int32_t* pValue) {};
void LibCpp::LittleEndian(int64_t* pValue) {};
#pragma GCC diagnostic warning "-Wunused-parameter"

void LibCpp::BigEndian(uint16_t* pValue) { BigEndian((int16_t*)pValue); }
void LibCpp::BigEndian(uint32_t* pValue) { BigEndian((int32_t*)pValue); }
void LibCpp::BigEndian(uint64_t* pValue) { BigEndian((int64_t*)pValue); }
void LibCpp::LittleEndian(uint16_t* pValue) { LittleEndian((int16_t*)pValue); }
void LibCpp::LittleEndian(uint32_t* pValue) { LittleEndian((int32_t*)pValue); }
void LibCpp::LittleEndian(uint64_t* pValue) { LittleEndian((int64_t*)pValue); }

string GetErrorString(unsigned long errorCode)
{
    char* txt = strerror(errorCode);
    return txt;
}

void LibCpp::sleep(int duration_ms)
{
    usleep(duration_ms * 1000);
}
