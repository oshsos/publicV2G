#ifndef CPLCSTACK_H
#define CPLCSTACK_H

#include "../../LibCpp/HAL/cDebug.h"
#include "../../LibCpp/HAL/cPacketSocket.h"
#include "../../LibCpp/HAL/cFramePort.h"

#include "slac_messages.h"
#include "cQcaMAC.h"

#define cPlcStack_BUFFERSIZE 256

namespace Iso15118
{

//typedef struct stSounding
//{
//    struct timeval ts;
//    struct timeval tc;
//    signed timer;
//    unsigned AAG [SLAC_GROUPS];
//    unsigned sounds;

//    stSounding();
//    void clear();
//} stSounding;

class cPlcStackPub : public LibCpp::iFramePort, public cQcaMAC
{
public:
    cPlcStackPub(LibCpp::cPacketSocket* pSocket);
    void clear();               ///< Clears all session relevant data.
    virtual char* getQcaMAC() {return qcaMAC;};

    bool checkMessage(std::string messageType, uint8_t mmv, uint8_t appType, uint8_t secType, uint8_t runId[SLAC_RUNID_LEN]);
    uint16_t evaluateMessage(char* receiveMessage, int receiveMessageLen);

    // host to qca chip
    void send_GET_SW_VER_REQ(); ///< Sends software version request (GET_SW_VER.REQ).
    void send_CM_SET_KEY_REQ();
    void send_VS_PL_LINK_STATUS_REQ();
    void send_VS_NW_INFO_REQ();
    void send_VS_PEER_PING_REQ();
    void send_VS_PEER_PING_RSP();

    // SECC homplug AV messages
    void send_CM_SLAC_PARAM_CNF();
    //void send_CM_START_ATTEN_CHAR_CNF();
    void send_CM_ATTEN_CHAR_IND();
    void send_CM_ATTEN_CHAR_RSP();
    void send_CM_SLAC_MATCH_CNF();

    // EVCC homplug AV messages
    void send_CM_SLAC_PARAM_REQ();
    void send_CM_START_ATTEN_CHAR_IND();
    void send_CM_MNBC_SOUND_IND(int soundNumDecrement);
    void send_CM_SLAC_MATCH_REQ();

    bool soundsComplete();
    bool connectionSuccessful();

private:
    void onFrameReceive(LibCpp::cFramePort* pFramePort);
    bool onReceiveFrameCheck(LibCpp::cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize);

    void initializeSession();   ///< Initializes the value struct prior to a session start.
    //void initializeSounding();   ///< Initializes the value struct prior to a sounding procedure.
    bool analyseSounding();   ///< Analyses the sounding data on EV side.
    void clearSendBuffer();     ///< clears the sendBuffer
    void fillEthernetHdr(LibCpp::stEthernetHeader* pHeader, char destinationMAC[ETHER_ADDR_LEN]);   ///< Fills the ethernet frame header (MAC addresses).
    void fillQualcommHdr(qualcomm_hdr* pHeader, uint16_t MMTYPE);                       ///< Fills the Homeplug AV header for Qualcomm manufacturer dependent messages.
    void fillHomePlugHdr(homeplug_fmi* pHeader, uint16_t MMTYPE);

public:
    LibCpp::cDebug              dbg;
    uint16_t                    lastEvaluationResult;
    session                     values;                             ///< Struct to store all parameters to be sentretrieved by the SLAC process

private:
    LibCpp::cPacketSocket*      pSocket;
    //stSounding                  sounding;
    char                        sendBuffer[cPlcStack_BUFFERSIZE];
    char                        qcaMAC[ETHER_ADDR_LEN];             ///< MAC address of the Qualcomm plc chip.
//    char*                       receiveMessage;
//    int                         receiveMessageLen;

    uint8_t                     sessionIdentNumber;
    unsigned int                paramRequestCount;                  ///< Counts the number of emitted SLAC_PARAM.REQ messages.
};

}

#endif
