// UTF8 (ü) //
/**
\file   cCRC32.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-02-21

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_Classes
@{

\class LibCpp::cCRC32

see:\n
https://de.wikipedia.org/wiki/Zyklische_Redundanzpr%C3%BCfung\n
https://github.com/Michaelangel007/crc32\n

**/

#ifndef CCRC32_H
#define CCRC32_H

#include <cstdint>

namespace LibCpp
{

/**
 * @brief Class for 32 bit cyclic redundancy check calculation
 */
class cCRC32
{
public:
    static constexpr uint32_t POLY32    = 0x04C11DB7;   ///< CRC32 defined polynom
    static constexpr uint32_t POLY32REV = 0xEDB88320;   ///< CRC32 polynom in reversed bit order (Ethernet standard)

    uint32_t static calculate(const char* buffer, unsigned int length, bool ethernetType = true);   ///< Check sum CRC32 calculation
    uint32_t static inline calculate(const uint8_t* buffer, unsigned int length, bool ethernetType = true) {return calculate((char*)buffer, length, ethernetType);}; ///< Check sum CRC32 calculation

private:
    char     static reverse(char value);        ///< Reverses the bit order
    uint32_t static reverse(uint32_t value);    ///< Reverses the bit order
};

}
#endif

/** @} */
