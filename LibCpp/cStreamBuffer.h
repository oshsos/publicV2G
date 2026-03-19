// UTF8 (ü) //
/**
\file   cStreamBuffer.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2025-11-10

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp
@{
\class LibCpp::cStreamBuffer

\brief The class is used as an intermediate class between a 'char' buffer and the istream and ostream classes.

This class contains the control to read or write data to an assigned buffer. It basically contains
read and write pointers as well as capacity information. Thus it supports istream and ostream operations
by supplying an appropriate buffer reference to these classes.\n
\n
The typical usage is as follows:
\code
    unsigned char buffer[256] = {6, 200, 78, 23, 95, 34, 98, 45, 22, 58, 22, 54, 67};
    cStreamBuffer bufferCtrl((char*)buffer, 256);
    ostream reader(&bufferCtrl);
    reader << 7;
\endcode
or more specific for e.g. non volatile memory operation:
\code
    unsigned char buffer[256] = {6, 200, 78, 23, 95, 34, 98, 45, 22, 58, 22, 54, 67};
    cStreamBuffer bufferCtrl((char*)buffer, 256);
    ostream writer(&bufferCtrl);
    writer.erase();
    reader << 7;
\endcode
The istream and ostream classes support just writing and reading binary 'char' values. This is different to
classes like ifstream which correctly accept integers or floats!\n
\n
In order to make own designed classes streamable you can define << and >> operators in the following manner.
\code
class myClass
{
  friend istream &operator>>(istream &in, myClass &c);
  friend ostream &operator<<(ostream &out, const myClass &c);
}
\endcode
 */

#ifndef CSTREAMBUFFER_H
#define CSTREAMBUFFER_H

#include <iostream>

namespace LibCpp
{

/** \brief Read and write control of an 'char' array buffer */
class cStreamBuffer : public std::basic_streambuf<char>
{
public:
    cStreamBuffer(char* buffer, size_t capacity);       ///< @copybrief cStreamBuffer::cStreamBuffer
    virtual ~cStreamBuffer();                           ///< @copybrief cStreamBuffer::~cStreamBuffer
    virtual void reset();                               ///< @copybrief cStreamBuffer::reset
    virtual void erase();                               ///< @copybrief cStreamBuffer::erase
};

}
#endif
/** @} */
