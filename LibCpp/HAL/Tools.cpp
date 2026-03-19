#define _CRT_SECURE_NO_WARNINGS         // required for Visual Studio compiler

#include "Tools.h"
#include "HW_Tools.h"
#include <cstdarg>

using namespace std;
using namespace LibCpp;

string LibCpp::ByteArrayToString(const char* pArray, int length)
{
    if (length <= 0) return "<empty>";
    string out = "0x ";
    for(int i=0; i<length; i++)
    {
        unsigned int value = (unsigned char) pArray[i];
        out += stringFormat("%02x ", value);
    }
    return out;
}

string LibCpp::ByteArrayToString(const uint8_t* pArray, int length)
{
    return ByteArrayToString((const char*)pArray, length);
}

cByteArray::cByteArray()
{
    pData = nullptr;
    dataSize = 0;
}

cByteArray::cByteArray(unsigned int size)
{
    pData = nullptr;
    dataSize = 0;
    set(size);
}

cByteArray::cByteArray(const char* pSourceData, unsigned int size)
{
    pData = nullptr;
    dataSize = 0;
    set(pSourceData, size);
}

cByteArray::cByteArray(const int64_t source, bool bigEndian)
{
    pData = nullptr;
    dataSize = 0;
    set(source, bigEndian);
}

cByteArray::cByteArray(const std::string source)
{
    pData = nullptr;
    dataSize = 0;
    set(source);
}

cByteArray::cByteArray(const cByteArray& source)
{
    pData = nullptr;
    dataSize = 0;
    set(source.pData, source.dataSize);
}

cByteArray::~cByteArray()
{
    if (pData)
        delete[] pData;
    pData = nullptr;
    dataSize = 0;
}

void cByteArray::clear()
{
    set();
}

void cByteArray::set(const unsigned int size)
{
    set(nullptr, size);
}

void cByteArray::set(const char* pSourceData, const unsigned int size)
{
    if (pData)
        delete[] pData;
    pData = nullptr;
    dataSize = 0;
    if (size)
    {
        pData = new char[size+1];
        dataSize = size;
        pData[size] = 0;

        if (pSourceData)
            for (unsigned int i=0; i<size; i++)
                pData[i] = pSourceData[i];
        else
            for (unsigned int i=0; i<size; i++)
                pData[i] = 0;
    }
}

void cByteArray::set(const uint64_t source, bool bigEndian)
{
    set((int64_t)source, bigEndian);
}

void cByteArray::set(const int64_t source, bool bigEndian)
{
    int64_t value = source;
    if (bigEndian)
        BigEndian(&value);
    else
        LittleEndian(&value);
    set((const char*)&value, 8);
}

void cByteArray::set(const std::string source)
{
    set(source.c_str(), (unsigned int)source.size());
}

void cByteArray::set(const cByteArray& source)
{
    set(source.pData, source.dataSize);
}

string cByteArray::toString()
{
    return ByteArrayToString(pData, dataSize);
}

string cByteArray::toAllTypesString()
{
    string out;
    out += toString() + stringFormat(" (%llu, '", asInt64(true)) + asString() + "')";
    return out;
}

void cByteArray::asChar(char* buffer, int bufferLen)
{
    if (!buffer) return;
    if (!pData || bufferLen<=0)
    {
        buffer[0] = 0;
        return;
    }
    if (bufferLen>(int)size())
        for (int i=size(); i<bufferLen; i++)
            buffer[i] = 0;
    for (unsigned int i=0; i<size(); i++)
        buffer[i] = pData[i];
}

string cByteArray::asString()
{
    if (!pData)
        return "";
    return string(pData);
}

uint64_t cByteArray::asUInt64(bool bigEndian)
{
    if (!pData)
        return 0;
    uint64_t value = *((uint64_t*)pData);
    if (bigEndian)
        BigEndian(&value);
    else
        LittleEndian(&value);
    if (dataSize<8)
        value >>= (8-dataSize)*8;
    return value;
}

int64_t cByteArray::asInt64(bool bigEndian)
{
    if (!pData)
        return 0;
    int64_t value = *((uint64_t*)pData);
    if (bigEndian)
        BigEndian(&value);
    else
        LittleEndian(&value);
    if (dataSize<8)
        value >>= (8-dataSize)*8;
    return value;
}

char* cByteArray::data()
{
    return pData;
}

unsigned int cByteArray::size()
{
    return dataSize;
}

char& cByteArray::operator [](unsigned int i)
{
    if (i >= dataSize) i = 0;
    return pData[i]; //(*(pData+i));
}

cByteArray& cByteArray::operator = (const cByteArray& source)
{
    if (this == &source) return *this;
    this->pData = nullptr;
    this->dataSize = 0;
    set(source.pData, source.dataSize);
    return *this;
}

bool cByteArray::operator == (const cByteArray& b)
{
    if (!pData && !b.pData) return true;
    if (!pData || !b.pData) return false;
    if (dataSize != b.dataSize) return false;
    for (unsigned int i=0; i<dataSize; i++)
        if (pData[i] != b.pData[i]) return false;
    return true;
}

void LibCpp::printf_flush(const char* format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    ::vprintf(format, arg_ptr);
    va_end(arg_ptr);
    ::fflush(stdout);
}

string LibCpp::stringFormat(const char* format, ...)
{
    char txt[256];
    va_list arg_ptr;
    va_start(arg_ptr, format);
    ::vsprintf(txt, format, arg_ptr);
    va_end(arg_ptr);
    return string(txt);
}

string LibCpp::stringTrim(string data)
{
    const string whitespace = " \t\f\v\n\r";
    string::size_type start = data.find_first_not_of(whitespace);
    string::size_type end = data.find_last_not_of(whitespace);
    if (start == string::npos) start = 0;
    if (end == string::npos) end = data.size()-1;
//    if (start == string::npos || end == string::npos)
//        return "";
    return data.substr(start, end-start+1);
}

vector<string> LibCpp::stringSplit(string data, string delimiter, bool doTrim, bool removeEmpty)
{
    vector<string> list;
    string::size_type start = 0;
    string::size_type end = data.find(delimiter);
    if (end == std::string::npos)
        list.push_back(data);
    while (end != std::string::npos)
    {
        list.push_back(data.substr(start, end-start));
        start = end + delimiter.size();
        if (start == data.size())
            list.push_back("");
        end = data.find(delimiter, start);
    }
    if (start < data.size())
        list.push_back(data.substr(start, data.size()-start));
    if (doTrim)
        for (unsigned int i=0; i<list.size(); i++)
            list[i] = stringTrim(list[i]);
    if (removeEmpty)
    {
        for (int i=list.size()-1; i>=0; i--)
            if (list[i].empty())
                list.erase(list.begin() + i);
    }
    return list;
}

