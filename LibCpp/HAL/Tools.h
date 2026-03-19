#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <cstdint>
#include <vector>

namespace LibCpp
{

std::string ByteArrayToString(const char* pArray, int length);
std::string ByteArrayToString(const uint8_t* pArray, int length);

class cByteArray
{
public:
    cByteArray();                                                   ///< Standardconstructor (required for standard initialization e.g. at 'new' calls)
    cByteArray(unsigned int size);                                  ///< Constructor for an empty array (all values set to zero)
    cByteArray(const char* sourceData, unsigned int size);
    cByteArray(const int64_t source, bool bigEndian);
    cByteArray(const std::string source);
    cByteArray(const cByteArray& source);
    ~cByteArray();
    void clear();
    void set(unsigned int size = 0);
    void set(const char* pSourceData, unsigned int size);
    void set(const int64_t source, bool bigEndian);
    void set(const uint64_t source, bool bigEndian);
    void set(const std::string source);
    void set(const cByteArray& source);
    void asChar(char* buffer, int bufferLen = -1);
    uint64_t asUInt64(bool bigEndian);
    int64_t asInt64(bool bigEndian);
    std::string asString();
    std::string toString();
    std::string toAllTypesString();
    char* data();
    unsigned int size();
    char& operator [](unsigned int i);
    cByteArray& operator = (const cByteArray& source);
    bool operator == (const cByteArray& b);

private:
    char* pData;
    unsigned int dataSize;
};

void        printf_flush(const char* format, ...);
std::string stringFormat(const char* format, ...);
std::string stringTrim(std::string data);
std::vector<std::string> stringSplit(std::string data, std::string delimiter = " ", bool doTrim = true, bool removeEmpty = false);

}
#endif
