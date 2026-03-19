/*
 * CANCommon.h
 *
 *  Created on: 12.03.2009
 *      Author: sanders
 */

#ifndef CANCOMMON_H_
#define CANCOMMON_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/time.h> // struct timeval

#ifdef  __cplusplus
}
#endif

typedef struct
{
	unsigned int node;			// CAN Node number
	unsigned int Id;			// CAN ID
	unsigned char data[8];		// Data Bytes
	unsigned char dlc;			// Data Length Code
    struct timeval timeStamp;	// received at
} SCanMsg;



#endif /*CANCOMMON_H_*/
