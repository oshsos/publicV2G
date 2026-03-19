// utf8 (ü)
/*
 * SocketCAN.h
 *
 *  Created on: 18.06.2010
 *      Author: sanders
 */

#ifndef cSocketCAN_H_
#define cSocketCAN_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <net/if.h>
#include <linux/can.h>
#include <sys/types.h>
#include <sys/socket.h>


#ifdef  __cplusplus
}
#endif

#include <vector>
#include "CANCommon.h"
#include "../../../../BibOb/HAL/obCanPort.h"

#ifdef PROJECTDEFS
  #include <ProjectDefs.h>
#endif

#ifndef  TIMEOUT_CAN
  #define TIMEOUT_CAN 10
#endif


class cSocketCAN
{
public:
	cSocketCAN();
	virtual ~cSocketCAN();

	int StartCANInterface(unsigned int nodeNumber, int bitrate);
	int StopCANInterface();
	int SetFilter(unsigned int id, unsigned int mask, int priority);
	int GetCANMessage(SCanMsg& canMsg, int timeout);
	//int GetCANMessages(std::vector<SCanMsg>& messages, int timeout);
	int SendCANMessage(SCanMsg& canMsg);


	//int SetCallBack(void(*)(void*),void*);
//	int Send(stCanMsg*,int);
//	int Receive(stCanMsg* pMsg,int Wait);
//	void Operate();
//	void GetCanState(enum CanState* pState, int Wait);
//
	int IntReceive(stCanMsg* pMsg);

	void printDLC(void * ptr);



private:
	int m_canSocket;
	int m_filterId;
	int m_frameBufferSize;
	struct sockaddr_can m_addr;
	struct ifreq m_ifr;
	struct can_frame* m_frameBuffer;
	struct mmsghdr* m_msgs;
	struct iovec* m_iovecs;
	char** m_ctrlmsg;
  SCanMsg   m_cMsg;

	cSocketCAN(const cSocketCAN &rhs);
	cSocketCAN &operator=(const cSocketCAN &rhs);

	int ReadMessages(SCanMsg& canMsg);

};

#endif /* SOCKETCAN_H_ */
