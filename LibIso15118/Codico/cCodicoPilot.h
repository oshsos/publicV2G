#ifndef CCODICOPILOT_H
#define CCODICOPILOT_H

#define MTYPE_CONTROLFRAME 0x6003

#define CF_VERSION 0x00
#define CF_TYPE_FRAMINGPROTOCOL 0x04

#define FP_SOF 0xC0
#define FP_EOF 0xC1

#define CP_MODULEID 0x29
#define CP_SUBID_SetMode 0x40
#define CP_SUBID_GetMode 0x41
#define CP_SUBID_Start 0x42
#define CP_SUBID_Stop 0x43
#define CP_SUBID_SetPwm 0x44
#define CP_SUBID_GetPwm 0x45
#define CP_SUBID_SetResistor 0x46
#define CP_SUBID_GetResistor 0x47
#define CP_SUBID_GetState 0x48
#define CP_SUBID_SetNotification 0x49
#define CP_SUBID_GetNotification 0x4A
#define CP_SUBID_GetAnalog 0x4B

#define CP_REQID_NOTIFICATION_ 0xFF
#define CP_SUBID_NOTIFICATION_PWM 0x80
#define CP_SUBID_NOTIFICATION_STATE 0x81

#define CD_SENDBUFFERSIZE 256

#include "../../LibCpp/HAL/cPacketSocket.h"
#include "../cControlPilot.h"
#include "../PlcStack/cQcaMAC.h"
#include "../types.h"
#include "../../LibCpp/cTimer.h"

namespace Iso15118
{

#pragma pack (push,1)

/** Header for Codico control frame packets */
typedef struct stControlFrameHeader
{
    uint8_t     version;                ///< Control frame version
    uint8_t     type;                   ///< Control frame type
    uint16_t    size;                   ///< Size of following payload
} stControlFrameHeader;

/** Header for Codico Framing Protocol\qn
After the payload a check sum byte and a end of frame byte follows. */
typedef struct stFramingProtocolHeader
{
    uint8_t     SOF;                    ///< Start of frame
    uint8_t     ModID;                  ///< Module ID (data destination)
    uint8_t     SubID;                  ///< Modules sub ID (data destination)
    uint8_t     ReqID;                  ///< Response message identifyer
    uint16_t    payloadSize;            ///< Size of following payload
} stFramingProtocolHeader;

typedef struct stFramingProtocolMsg
{
    stFramingProtocolHeader header;     ///< Header
    uint8_t     payload[1];             ///< First byte of message payload
} stFramingProtocolMsg;

typedef struct stFramingProtocolEtherMsg
{
    LibCpp::stEthernetHeader    ethernetHeader;
    stControlFrameHeader        controlFrameHeader;
    stFramingProtocolMsg        framingMessage;
} stFramingProtocolEtherMsg;

#pragma pack (pop)

enum class enCodicoResult : uint8_t
{
    success = 0,
    busy = 1,
    subIdUnknown = 2,
    commandUnknown = 3,
    malformed = 4,
    unexpected = 5,
    internalError = 0xFF
};

std::string enCodicoResult_toString(enCodicoResult result);

class cCodicoPilot : public cControlPilot, public LibCpp::iFramePort
{
public:
    cCodicoPilot(Iso15118::enStackType stackType, LibCpp::cPacketSocket* pSocket, Iso15118::cQcaMAC* pPlc);
    virtual ~cCodicoPilot();

    inline  void        setStackMode(Iso15118::enStackType stackType) {this->stackType = stackType;};

    virtual stCpValue   get();                  ///< Retrieves the actual state
    virtual void        set(stCpValue value);   ///< Sets the desired state
    virtual stCpValue   getDemand();            ///< Retrieves the last set desired state
    virtual enPpCode    getPpCode();            ///< Retrieves the cable current capacity according to the PP-code
    virtual void        operate();              ///< Synchronous state and communication processing

    void setAddress(uint8_t addressMAC[ETHER_ADDR_LEN]);
    bool isInitialized();

    void getCmdMode();
    void setCmdMode();
    void getCmdPwm();
    void setCmdPwm(uint8_t pwm);
    void startService();
    void stopService();
    void getCmdState();
    void setCmdState(uint8_t state);

    void evaluateMessage(char* message, int len);

private:
    void fillFramingProtocolFooter(stFramingProtocolHeader* pFrameHeader);
    unsigned int fillFramingProtocolEtherHeader(stFramingProtocolEtherMsg* pFramingEtherMsg);
    void sendFramingMessage(uint8_t subId, uint8_t* data = nullptr, int len = 0);

    void onFrameReceive(LibCpp::cFramePort* pPort);
    bool onReceiveFrameCheck(LibCpp::cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize = 0); ///< External method to check input stream data for completed frames and to manipulate received data

    static enCpState toCpState(int state);
    static int fromCpState(enCpState state);


public:
    LibCpp::cDebug dbg;

private:
    LibCpp::cPacketSocket* pSocket;
    Iso15118::cQcaMAC* pPlc;
    Iso15118::enStackType stackType;
    uint8_t destinationAddress[ETHER_ADDR_LEN];
    int initializationState;
    int receiveState;
    stCpValue value;
    LibCpp::cTimer pollTimer;
};

}
#endif
