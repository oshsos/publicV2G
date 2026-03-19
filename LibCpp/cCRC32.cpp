// UTF8 (ü) //
/**
\file cCRC32.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-02-21

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Classes
@{

\class LibCpp::cCRC32

**/

#include "cCRC32.h"

using namespace LibCpp;

/**
 * @brief Returns a char value in a reversed bit order
 * @param value
 * @return
 */
char cCRC32::reverse(char value)
{
    char result = 0;
    unsigned char mask = 1;
    unsigned char setter = 0x80;
    for (int i=0; i<32; i++)
    {
        if (value & mask) result |= setter;
        mask <<= 1;
        setter >>= 1;
    }
    return result;
}

/**
 * @brief Returns a long value in a reversed bit order
 * @param value
 * @return
 */
uint32_t cCRC32::reverse(uint32_t value)
{
    uint32_t result = 0;
    uint32_t mask = 1;
    uint32_t setter = 0x80000000;
    for (int i=0; i<32; i++)
    {
        if (value & mask) result |= setter;
        mask <<= 1;
        setter >>= 1;
    }
    return result;
}

/**
 * @brief Calculates the CRC-32 cyclic redundancy check value
 * @param buffer Input byte frame
 * @param length Length of the frame
 * @param ethernetType Uses the standard Ethernet algorithm modifications if set to 'true'.
 * @return CRC-32 value in system byte order
 */
uint32_t cCRC32::calculate(const char* buffer, unsigned int length, bool ethernetType)
{
    if (ethernetType)
    {
        uint32_t crc = ~0;

        for (unsigned int j=0; j<length; j++)
        {
            // char value = reverse(buffer[j]);
            crc = crc ^ (reverse(buffer[j]) << 24);
            for (int i=0; i<8; i++)
            {
                if (crc & 0x80000000)
                    crc = (crc << 1) ^ POLY32;
                else
                    crc = (crc << 1);
            }
        }
        return reverse(~crc);
    }
    else
    {
        uint32_t crc = ~0;

        for (unsigned int j=0; j<length; j++)
        {
            crc = crc ^ (buffer[j] << 24);
            for (int i=0; i<8; i++)
            {
                if (crc & 0x80000000)
                    crc = (crc << 1) ^ POLY32;
                else
                    crc = (crc << 1);
            }
        }
        return ~crc;
    }
}

/** @} */
