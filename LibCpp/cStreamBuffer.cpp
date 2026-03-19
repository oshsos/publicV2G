// UTF8 (ü) //
/**
\file cStreamBuffer.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-11-10

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp
@{
**/

#include "cStreamBuffer.h"

using namespace std;
using namespace LibCpp;

/** \brief Constructs a streaming control instance for a 'char' buffer.
 *  Read and write pointers will point to the begin of the buffer.
 */
cStreamBuffer::cStreamBuffer(char* buffer, size_t capacity)
{
    setg(buffer, buffer, buffer + capacity);
    setp(buffer, buffer + capacity);
}

/** \brief Destructor. */
cStreamBuffer::~cStreamBuffer()
{
}

/**
 * \brief Sets the read index to the begin of the buffer.
 */
void cStreamBuffer::reset()
{
//    istream rd(this);
//    rd.seekg(0);
    _M_in_cur = _M_in_beg;
}

/** \brief Sets the write position to the begin of the buffer and erases a non volatile memory like a Flash, if required.
 *  This method does not initialize the content of the buffer, it might contain arbitray values.
 */
void cStreamBuffer::erase()
{
    _M_out_cur = _M_out_beg;
}

/** @} */
