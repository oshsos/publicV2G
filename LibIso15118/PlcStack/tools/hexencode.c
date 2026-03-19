/*====================================================================*
 *
 *   size_t hexencode (void * memory, size_t extent, char const * string);
 *
 *   memory.h
 *
 *   encode a hexadecimal string into a fixed length memory region;
 *   return the number of bytes encoded or 0 on error; an error will
 *   occur of the entire region cannot be encoded or the entire
 *   string cannot be converted due to illegal or excessive digits;
 *
 *   permit an optional HEX_EXTENDER character between successive
 *   octets; constant character HEX_EXTENDER is defined in number.h;
 *
 *   Motley Tools by Charles Maier;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/
//
// Transforms a string containing a textual hex expressed number like "0a04" (having 4 bytes)
// into the corresponding byte array: char data[2] = {10, 4}
// The textual string may also be written as "0a:04"

#ifndef HEXENCODE_SOURCE
#define HEXENCODE_SOURCE

#define RADIX_HEX 16
#define HEX_DIGITS 2
#define HEX_EXTENDER ':'

#include <errno.h>
#include <ctype.h>
#include <stdint.h>

typedef uint8_t byte;

//#include "../tools/memory.h"
//#include "../tools/number.h"

unsigned char todigit (const unsigned char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return (c - '0');
    }
    if ((c >= 'A') && (c <= 'Z'))
    {
        return (c - 'A' + 10);
    }
    if ((c >= 'a') && (c <= 'z'))
    {
        return (c - 'a' + 10);
    }
    return (0xFF);
}

unsigned int hexencode (void * memory, unsigned int extent, char const * string)
{
    register byte * origin = (byte *)(memory);
	register byte * offset = (byte *)(memory);
	unsigned radix = RADIX_HEX;
	unsigned digit = 0;
	while ((extent) && (*string))
	{
		unsigned field = HEX_DIGITS;
		unsigned value = 0;
		if ((offset > origin) && (*string == HEX_EXTENDER))
		{
			string++;
		}
		while (field--)
		{
			if ((digit = todigit (*string)) < radix)
			{
				value *= radix;
				value += digit;
				string++;
				continue;
			}
			errno = EINVAL;
			return (0);
		}
		*offset = value;
		offset++;
		extent--;
	}

//#if defined (WIN32)

//	while (isspace (*string))
//	{
//		string++;
//	}

//#endif

	if ((extent) || (*string))
	{
		errno = EINVAL;
		return (0);
	}
	return (unsigned int)(offset - origin);
}


#endif
