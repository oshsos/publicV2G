// UTF8 (ü) //
/**
\file   HW_Tools.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-01

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_GLOB
@{

\page Page_LibCpp_HAL_HWTools Global hardware dependent functions

This header file defines a set of global functions being dependent of the actual
hardware they are used on. Therefore a corresponding source code file 'HW_Tools.cpp'
does exist for each supported hardware.\n
\n
A set of functions changes the byte order in memory from or to big endian or little endian.
In case the hardware does not provide this byte order naturally, the byte order
will be switched to or from the requested order.

A direct switch in memory is done by:

- BigEndian(int16_t*)
- BigEndian(int32_t*)
- BigEndian(int64_t*)
- BigEndian(uint16_t*)
- BigEndian(uint32_t*)
- BigEndian(uint64_t*)
- LittleEndian(int16_t*);
- LittleEndian(int32_t*);
- LittleEndian(int64_t*);
- LittleEndian(uint16_t*);
- LittleEndian(uint32_t*);
- LittleEndian(uint64_t*);

A new value in requested byte order is returned by:

- BigEndian(int16_t)
- BigEndian(int32_t)
- BigEndian(int64_t)
- BigEndian(uint16_t)
- BigEndian(uint32_t)
- BigEndian(uint64_t)
- LittleEndian(int16_t);
- LittleEndian(int32_t);
- LittleEndian(int64_t);
- LittleEndian(uint16_t);
- LittleEndian(uint32_t);
- LittleEndian(uint64_t);

**/

#ifndef HW_TOOLS_H
#define HW_TOOLS_H

#include <string>
#include <cstdint>

namespace LibCpp
{

void BigEndian(int16_t* pValue);        ///< Switches byte order in memory to or from  big endian
void BigEndian(int32_t* pValue);        ///< Switches byte order in memory to or from  big endian
void BigEndian(int64_t* pValue);        ///< Switches byte order in memory to or from  big endian
void BigEndian(uint16_t* pValue);       ///< Switches byte order in memory to or from  big endian
void BigEndian(uint32_t* pValue);       ///< Switches byte order in memory to or from  big endian
void BigEndian(uint64_t* pValue);       ///< Switches byte order in memory to or from  big endian
void LittleEndian(int16_t* pValue);     ///< Switches byte order in memory to or from  little endian
void LittleEndian(int32_t* pValue);     ///< Switches byte order in memory to or from  little endian
void LittleEndian(int64_t* pValue);     ///< Switches byte order in memory to or from  little endian
void LittleEndian(uint16_t* pValue);    ///< Switches byte order in memory to or from  little endian
void LittleEndian(uint32_t* pValue);    ///< Switches byte order in memory to or from  little endian
void LittleEndian(uint64_t* pValue);    ///< Switches byte order in memory to or from  little endian

inline int16_t BigEndian(int16_t value){BigEndian(&value); return value;};          ///< Retruns a value in big endian byte order
inline int32_t BigEndian(int32_t value){BigEndian(&value); return value;};          ///< Retruns a value in big endian byte order
inline int64_t BigEndian(int64_t value){BigEndian(&value); return value;};          ///< Retruns a value in big endian byte order
inline uint16_t BigEndian(uint16_t value){BigEndian(&value); return value;};        ///< Retruns a value in big endian byte order
inline uint32_t BigEndian(uint32_t value){BigEndian(&value); return value;};        ///< Retruns a value in big endian byte order
inline uint64_t BigEndian(uint64_t value){BigEndian(&value); return value;};        ///< Retruns a value in big endian byte order
inline int16_t LittleEndian(int16_t value){LittleEndian(&value); return value;};    ///< Retruns a value in little endian byte order
inline int32_t LittleEndian(int32_t value){LittleEndian(&value); return value;};    ///< Retruns a value in little endian byte order
inline int64_t LittleEndian(int64_t value){LittleEndian(&value); return value;};    ///< Retruns a value in little endian byte order
inline uint16_t LittleEndian(uint16_t value){LittleEndian(&value); return value;};  ///< Retruns a value in little endian byte order
inline uint32_t LittleEndian(uint32_t value){LittleEndian(&value); return value;};  ///< Retruns a value in little endian byte order
inline uint64_t LittleEndian(uint64_t value){LittleEndian(&value); return value;};  ///< Retruns a value in little endian byte order

std::string GetErrorString(unsigned long errorCode);

void sleep(int duration_ms);

}
#endif

/** @} */
