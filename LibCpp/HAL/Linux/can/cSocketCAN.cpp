// utf8 (ü)
/*
 * SocketCAN.cpp
 *
 *  Created on: 18.06.2010
 *      Author: sanders
 */

#include "cSocketCAN.h"
#include <cstring>

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include <linux/can/raw.h>
#include <pthread.h>


#ifdef  __cplusplus
}
#endif

#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

//static const unsigned int CTRL_MSG_SIZE = CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32));
static const unsigned int CTRL_MSG_SIZE = CMSG_SPACE(sizeof(struct timeval));

cSocketCAN::cSocketCAN() :
	m_canSocket(0),
	m_filterId(0),
	m_frameBufferSize(64)
{
	m_frameBuffer = new struct can_frame[m_frameBufferSize];
	m_msgs = new struct mmsghdr[m_frameBufferSize];
	m_iovecs = new struct iovec[m_frameBufferSize];
	m_ctrlmsg = new char*[m_frameBufferSize];

	memset(m_msgs, 0, sizeof(struct mmsghdr));
	for (int i = 0; i < m_frameBufferSize; i++)
	{
		m_iovecs[i].iov_base         = &m_frameBuffer[i];
		m_iovecs[i].iov_len          = sizeof(struct can_frame);
		m_msgs[i].msg_hdr.msg_iov    = &m_iovecs[i];
		m_msgs[i].msg_hdr.msg_iovlen = 1;
		m_ctrlmsg[i] = new char[CTRL_MSG_SIZE];
		m_msgs[i].msg_hdr.msg_control = m_ctrlmsg[i];
	}

	memset(&m_cMsg,0,sizeof(m_cMsg));
}

cSocketCAN::~cSocketCAN()
{
	if (m_canSocket)
	{
		StopCANInterface();
	}

	if (m_frameBuffer)
	{
		delete[] m_frameBuffer;
	}

	if (m_msgs)
	{
		delete[] m_msgs;
	}

	if (m_iovecs)
	{
		delete[] m_iovecs;
	}

	if (m_ctrlmsg)
	{
		for (int i = 0; i < m_frameBufferSize; i++)
		{
			if (m_ctrlmsg[i])
			{
				delete[] m_ctrlmsg[i];
			}
		}
		delete[] m_ctrlmsg;
	}

}


int cSocketCAN::StartCANInterface(unsigned int nodeNumber, int bitrate)
{
#ifdef ARM_PLATFORM
	static const char* nodeNames[6] = {"hcan0", "hcan1", "hcan2", "hcan3", "hcan4", "hcan5"};
#else
	static const char* nodeNames[6] = {"can0", "can1", "can2", "can3", "can4", "can5"};
#endif

	if(nodeNumber < 0 || nodeNumber > 5)
	{
		fprintf(stderr, "Error [h]can%d not supported\n", nodeNumber);
		return 0;
	}

	if(bitrate > 1000)
	{
		bitrate = bitrate/1000;
	}

	/* open socket */
	errno = 0;
	if ((m_canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		fprintf(stderr, "Error on open Socket: %s\n", strerror(errno));
		return 0;
	}

	/* get node index */
	m_addr.can_family = AF_CAN;
	strncpy(m_ifr.ifr_name, nodeNames[nodeNumber], IFNAMSIZ);
	errno = 0;
	if (ioctl(m_canSocket, SIOCGIFINDEX, &m_ifr) < 0)
	{
		perror("SIOCGIFINDEX");
		fprintf(stderr, "Error on SIOCGIFINDEX: %s\n", strerror(errno));
		close(m_canSocket);
		m_canSocket = 0;
		return 0;
	}
	m_addr.can_ifindex = m_ifr.ifr_ifindex;

	const int timestamp_on = 1;
	if (setsockopt(m_canSocket, SOL_SOCKET, SO_TIMESTAMP, &timestamp_on, sizeof(timestamp_on)) < 0)
	{
		perror("setsockopt SO_TIMESTAMP");
		close(m_canSocket);
		m_canSocket = 0;
		return 0;
	}

	/* bind */
	errno = 0;
	if (bind(m_canSocket, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		fprintf(stderr, "Error on bind Socket: %s\n", strerror(errno));
		close(m_canSocket);
		m_canSocket = 0;
		return 0;
	}

	return 1;
}

int cSocketCAN::StopCANInterface()
{
	if (m_canSocket >= 0)
	{
		/* close socket */
		close(m_canSocket);
		m_canSocket = 0;
	}

	return 1;
}

int cSocketCAN::SetFilter(unsigned int id, unsigned int mask, int priority)
{
	/* set socket can filter too */
	struct can_filter rfilter;
	rfilter.can_id = id; /* SFF frame */
	rfilter.can_mask = mask;
	setsockopt(m_canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

	/* disable local loop back */
	int loopback = 0;
	setsockopt(m_canSocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

	return 1;
}

int cSocketCAN::SendCANMessage(SCanMsg& canMsg)
{
	static struct can_frame cf;
	static int nbytes = 0;

	memset(&cf, 0, sizeof(struct can_frame));

	cf.can_id = canMsg.Id;
	if (canMsg.Id > 0x7FF) // EFF
	{
		cf.can_id |= CAN_EFF_FLAG;
	}

	cf.can_dlc = canMsg.dlc;
	memcpy(cf.data, canMsg.data, 8);

	errno = 0;
	/* send frame */
	if ((nbytes = write(m_canSocket, &cf, sizeof(cf))) != sizeof(cf))
	{
		perror("write");
	}

	return nbytes;
}

int cSocketCAN::GetCANMessage(SCanMsg& canMsg, int timeout)
{
  int rval;
    fd_set  set;
    struct timeval tv;
    struct timeval start;
    struct timeval stop;
    struct timeval diff;
    int mdiff;

    canMsg.dlc = 0;

    FD_ZERO(&set);
    FD_SET(m_canSocket, &set);

      tv.tv_sec = timeout/1000;
      tv.tv_usec = (timeout%1000)*1000;

      gettimeofday(&start,(struct timezone *)0);

      do
      {
      rval = select((m_canSocket+1), &set, NULL, NULL, &tv);
      if (rval == -1)
      {
        perror("select()");
      }
      else if (rval)
      {
        if(m_canSocket && FD_ISSET(m_canSocket, &set))
        {
          if(ReadMessages(canMsg) > 0)
          {
            break;
          }
        }
      }
  //    else
  //    {
  //      printf("timeout.\n");
  //    }

      gettimeofday(&stop,(struct timezone *)0);
      timersub(&stop, &start, &diff);
        mdiff = (diff.tv_sec*1000) + (diff.tv_usec/1000);

      } while(mdiff < timeout);



    return canMsg.dlc;
}

//int cSocketCAN::GetCANMessages(std::vector<SCanMsg>& messages, int timeout)
//{
//	int rval = 0;
//	struct timespec tv;
//	struct can_frame* frame = 0;
//	struct cmsghdr *cmsg;
//
//    tv.tv_sec = timeout/1000;
//    tv.tv_nsec = (timeout%1000)*1000000;
//
//    memset(m_frameBuffer, 0, m_frameBufferSize*sizeof(struct can_frame));
//	for (int i = 0; i < m_frameBufferSize; i++)
//	{
//		m_iovecs[i].iov_len = sizeof(struct can_frame);
//		m_msgs[i].msg_hdr.msg_controllen = CTRL_MSG_SIZE;
//	}
//
//	errno = 0;
//	rval = recvmmsg(m_canSocket, m_msgs, m_frameBufferSize, 0, &tv);
//	if (rval == -1)
//	{
//		perror("recvmmsg()");
//		return 0;
//	}
//
////	printf("%d messages received\n", rval);
//	for (int i = 0; i < rval; i++)
//	{
//		frame = &m_frameBuffer[i];
//		SCanMsg canMsg;
//		memset(&canMsg, 0, sizeof(SCanMsg));
//
////		for (cmsg = CMSG_FIRSTHDR(&m_msgs[i].msg_hdr); cmsg && (cmsg->cmsg_level == SOL_SOCKET); cmsg = CMSG_NXTHDR(&m_msgs[i].msg_hdr,cmsg))
////		{
////			if (cmsg->cmsg_type == SO_TIMESTAMP)
////			{
////				canMsg.timeStamp = *(struct timeval *)CMSG_DATA(cmsg);
////			}
////		}
//		cmsg = CMSG_FIRSTHDR(&m_msgs[i].msg_hdr);
//		canMsg.timeStamp = *(struct timeval *)CMSG_DATA(cmsg);
//
//		canMsg.Id = frame->can_id & CAN_EFF_MASK;
//		canMsg.dlc = (frame->can_dlc > 8)?8:frame->can_dlc;
//		memcpy(canMsg.data, frame->data, 8);
//
//		messages.push_back(canMsg);
//	}
//
//	return rval;
//}


int cSocketCAN::ReadMessages(SCanMsg& canMsg)
{
  struct can_frame* frame = 0;
    struct timeval received;
    int Nframes = 0;

    memset(m_frameBuffer, 0, m_frameBufferSize*sizeof(struct can_frame));

    errno = 0;
    int count = read(m_canSocket, m_frameBuffer, m_frameBufferSize * sizeof(struct can_frame));
    if (count < 0)
    {
      if (errno == EAGAIN)
      {
        //printf("nothing to read\n");
      }
      else
      {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
      }
    }
    else if (count > 0)
    {
      Nframes = count/sizeof(struct can_frame);

      gettimeofday(&received,(struct timezone *)0);

      for (int i=0; i<Nframes; i++)
      {
        frame = &m_frameBuffer[i];

        //XXX Hier nochmal drüber, die alte config lies nur das lesen einer Nachricht zu...
        if ( 1 /*0 == canMsg.Id*/) // read any message
        {
          canMsg.Id = frame->can_id & CAN_EFF_MASK;
          canMsg.dlc = (frame->can_dlc > 8)?8:frame->can_dlc;
          canMsg.timeStamp.tv_sec = received.tv_sec;
          canMsg.timeStamp.tv_usec = received.tv_usec;
          memcpy(&canMsg.data, frame->data, 8);
        }
        else if (canMsg.Id == (frame->can_id & CAN_EFF_MASK)) // read specific message only
        {
          canMsg.dlc = (frame->can_dlc > 8)?8:frame->can_dlc;
          canMsg.timeStamp.tv_sec = received.tv_sec;
          canMsg.timeStamp.tv_usec = received.tv_usec;
          memcpy(&canMsg.data, frame->data, 8);
        }
      }
    }

    return canMsg.dlc;
}


int cSocketCAN::IntReceive(stCanMsg* pMsg) //,int* pn16MsgInBuffer)
{

  //if(!*pn16MsgInBuffer)
    {
    if(GetCANMessage(m_cMsg,TIMEOUT_CAN))				// Achtung: GetCANMessage kann Nachrichtenlänge = 0 nicht verarbeiten!
      {
      //RTR-Nachricht?
      if(m_cMsg.Id & RTR_FLAG)
        {
          //RTR-Nachricht
           pMsg->Len = m_cMsg.dlc;
           pMsg->ID  = (short)m_cMsg.Id; //XXX Fragwürdig da short nut 16 bit hat und bei Extended ID nicht reichen würden, besser hier unsigned int?
           pMsg->Len = 1;
           //*pn16MsgInBuffer = 1;

        }
      else
        {
          //Standart-Nachricht
          pMsg->Len = m_cMsg.dlc;
          pMsg->ID  = (short)m_cMsg.Id; //XXX Fragwürdig da short nut 16 bit hat und bei Extended ID nicht reichen würden, besser hier unsigned int?
          pMsg->RTR = 0;
          memcpy(&pMsg->Data,&m_cMsg.data,8);

          //*pn16MsgInBuffer = 1;
        }
      return 1;
      }

    }

  return 0; //return *pn16MsgInBuffer;
}





