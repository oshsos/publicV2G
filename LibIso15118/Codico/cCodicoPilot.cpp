#include <string>
#include <cstring>

#include "cCodicoPilot.h"

#include "../../LibCpp/HAL/HW_Tools.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

std::string Iso15118::enCodicoResult_toString(enCodicoResult result)
{
    switch (result)
    {
    case enCodicoResult::success: return "Success";
    case enCodicoResult::busy: return "Busy";
    case enCodicoResult::subIdUnknown: return "Sub ID unknown";
    case enCodicoResult::commandUnknown: return "Command unknown";
    case enCodicoResult::malformed: return "Malformed";
    case enCodicoResult::unexpected: return "Unexpected";
    case enCodicoResult::internalError: return "Internal error";
    default: return "<unknown " + to_string((uint8_t)result) + ">";
    }
}

uint8_t checksum(uint8_t aucData[], uint16_t usSize)
{
    uint32_t ulChecksum = 0U;
    uint16_t i;
    for (i = 0U; i < usSize; ++i)
    {
        ulChecksum += aucData[i];
    }
    ulChecksum = (ulChecksum & 0xFFFF) + (ulChecksum >> 16);
    ulChecksum = (ulChecksum & 0xFF) + (ulChecksum >> 8);
    ulChecksum = (ulChecksum & 0xFF) + (ulChecksum >> 8);
    if (ulChecksum != 0xFF)
    {
        ulChecksum = ~ulChecksum;
    }
    return (uint8_t)ulChecksum;
}

cCodicoPilot::cCodicoPilot(enStackType stackType, cPacketSocket* pSocket, cQcaMAC* pPlc) :
    dbg("cCodicoPilot"),
    pSocket(pSocket),
    pPlc(pPlc),
    stackType(stackType),
    pollTimer(500)
{
    initializationState = 0;
    receiveState = 0;
    memset(destinationAddress, 0, ETHER_ADDR_LEN);
//    destinationAddress[0]=0xc4;
//    destinationAddress[1]=0x93;
//    destinationAddress[2]=0x00;
//    destinationAddress[3]=0x48;
//    destinationAddress[4]=0xac;
//    destinationAddress[5]=0x7b; // 0x7d
    this->pSocket->setCallback(this);
}

void cCodicoPilot::setAddress(uint8_t addressMAC[ETHER_ADDR_LEN])
{
    memcpy(destinationAddress, addressMAC, ETHER_ADDR_LEN);
}

cCodicoPilot::~cCodicoPilot()
{
}

bool cCodicoPilot::isInitialized()
{
    return initializationState == 4;
}

void cCodicoPilot::fillFramingProtocolFooter(stFramingProtocolHeader* pFrameHeader)
{
    uint16_t len = sizeof(stFramingProtocolHeader) + BigEndian(pFrameHeader->payloadSize);
    uint8_t* pCrc = (uint8_t*)pFrameHeader + len;
    uint8_t* pEof = pCrc + 1;
    len += 2;

    *pCrc = 0;
    *pEof = FP_EOF;

    *pCrc = checksum((uint8_t*)pFrameHeader, len);
    if (!*pCrc) *pCrc = (uint8_t)0xFF;
}

unsigned int cCodicoPilot::fillFramingProtocolEtherHeader(stFramingProtocolEtherMsg* pFramingEtherMsg)
{
    pFramingEtherMsg->controlFrameHeader.version = CF_VERSION;
    pFramingEtherMsg->controlFrameHeader.type = CF_TYPE_FRAMINGPROTOCOL;
    pFramingEtherMsg->controlFrameHeader.size = BigEndian( (uint16_t)(sizeof(stFramingProtocolHeader) + BigEndian(pFramingEtherMsg->framingMessage.header.payloadSize) + 2));

    memcpy(pFramingEtherMsg->ethernetHeader.ODA, destinationAddress, ETHER_ADDR_LEN);
    memcpy(pFramingEtherMsg->ethernetHeader.OSA, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN);
    pFramingEtherMsg->ethernetHeader.MTYPE = BigEndian((uint16_t)MTYPE_CONTROLFRAME);

    return sizeof(stEthernetHeader) + sizeof(stControlFrameHeader) + BigEndian( pFramingEtherMsg->controlFrameHeader.size );
}

void cCodicoPilot::sendFramingMessage(uint8_t subId, uint8_t* data, int len)
{
    uint8_t sendBuffer[CD_SENDBUFFERSIZE];
    stFramingProtocolEtherMsg* pMsg = (stFramingProtocolEtherMsg*)sendBuffer;
    stFramingProtocolHeader* pHeader = &pMsg->framingMessage.header;
    uint8_t* pData = pMsg->framingMessage.payload;

    pHeader->SOF = FP_SOF;
    pHeader->ModID = CP_MODULEID;
    pHeader->SubID = subId;
    pHeader->ReqID = 0;
    pHeader->payloadSize = BigEndian((uint16_t)len);

    if (data && len)
        for (int i=0; i<len; i++)
            pData[i] = data[i];

    fillFramingProtocolFooter(pHeader);
    unsigned int length = fillFramingProtocolEtherHeader(pMsg);

    for (int i=length; i<60; i++) sendBuffer[i] = 0;
    if (length<60) length = 60;
    pSocket->send(sendBuffer, length);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief External method to check input stream data for completed frames and to manipulate received data
 * @param pFramePort
 * @param receiveBuffer
 * @param pLen
 * @param bufferSize
 * @return
 */
bool cCodicoPilot::onReceiveFrameCheck(cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize)
{
    stEthernetHeader* pHdr = (stEthernetHeader*)receiveBuffer;
    if (pHdr->MTYPE == BigEndian((uint16_t)MTYPE_CONTROLFRAME))
        return true;
    return false;
}

void cCodicoPilot::onFrameReceive(cFramePort* pFramePort)
{
    cDebug dbg("onFrameReceive", &this->dbg);

    // dbg.printf(LibCpp::enDebugLevel_Debug, "Callback receive.");

    char* receiveMessage = nullptr;
    int receiveMessageLen = pFramePort->receiveBuffer(&receiveMessage);

    if (receiveMessageLen)
    {
        stEthernetHeader* pHdr = (stEthernetHeader*)receiveMessage;
        if (pHdr->MTYPE == BigEndian((uint16_t)MTYPE_CONTROLFRAME))
            evaluateMessage(receiveMessage, receiveMessageLen);
    }

    pFramePort->receiveAcknowledge();
    return;
}

void cCodicoPilot::evaluateMessage(char* message, int len)
{
    cDebug dbg("evaluateMessage", &this->dbg);

    stFramingProtocolEtherMsg* pMsg = (stFramingProtocolEtherMsg*)message;
    stFramingProtocolHeader* pHeader = &pMsg->framingMessage.header;
    uint8_t* pData = pMsg->framingMessage.payload;

    if (pMsg->ethernetHeader.MTYPE != BigEndian((uint16_t)MTYPE_CONTROLFRAME))
        return;

    if (pHeader->ModID != CP_MODULEID)
    {
        dbg.printf(LibCpp::enDebugLevel_Debug, "Received Codico message from unknown module!");
        return;
    }

    switch(pHeader->SubID)
    {
    case CP_SUBID_GetMode:
    {
        enCodicoResult result = (enCodicoResult)pData[0];
        bool isEvse = pData[1];
        dbg.printf(enDebugLevel_Debug, "Received from Codico CP SetMode: Result = %s, EVSE Mode = %s.", enCodicoResult_toString(result).c_str(), isEvse ? "true" : "false");
    } break;
    case CP_SUBID_SetMode:
    {
        enCodicoResult result = (enCodicoResult)*pData;
        dbg.printf(enDebugLevel_Debug, "Received from Codico CP SetMode: Result = %s.", enCodicoResult_toString(result).c_str());
        if (result == enCodicoResult::success)
        {
            receiveState = initializationState;
        }
        else
        {
            dbg.printf(enDebugLevel_Fatal, "Set mode to %s failed! Check if the correct Codico hardware (EV or EVSE) is connected!", enStackType_toString(stackType).c_str());
        }
    } break;
    case CP_SUBID_GetPwm:
    {
        enCodicoResult result = (enCodicoResult)pData[0];
        uint16_t* pPwm = (uint16_t*)&pData[1];
        unsigned int pwm = (BigEndian(*pPwm) + 5) / 10;
        dbg.printf(enDebugLevel_Debug, "Received from Codico CP GetPwm: Result = %s, PWM = %i%%.", enCodicoResult_toString(result).c_str(), (int)pwm);
        if (result == enCodicoResult::success)
            value.pwm = enCpPwm_decode(pwm);
    } break;
    case CP_SUBID_SetPwm:
    {
        enCodicoResult result = (enCodicoResult)*pData;
        dbg.printf(enDebugLevel_Debug, "Received from Codico CP SetPwm: Result = %s.", enCodicoResult_toString(result).c_str());
        if (result == enCodicoResult::success)
        {
            receiveState = initializationState;
        }
    } break;
    case CP_SUBID_Start:
    {
        enCodicoResult result = (enCodicoResult)*pData;
        dbg.printf(enDebugLevel_Debug, "Received startService: Result = %s.", enCodicoResult_toString(result).c_str());
        if (result == enCodicoResult::success)
        {
            receiveState = initializationState;
        }
    } break;
    case CP_SUBID_Stop:
    {
        enCodicoResult result = (enCodicoResult)*pData;
        dbg.printf(enDebugLevel_Debug, "Received from Codico CP Stop: Result = %s.", enCodicoResult_toString(result).c_str());
    } break;
    case CP_SUBID_GetState:
    {
        // 12V:state A=0, 9V: state B=1, 6V: state C=2, 3V: state D=3, 0V: state E=4, -12V: state F=5, unknown=6.
        enCodicoResult result = (enCodicoResult)pData[0];
        int state = (int)pData[1];
        //dbg.printf(enDebugLevel_Debug, "Received from Codico CP State: Result = %s, State = %i.", enCodicoResult_toString(result).c_str(), state);
        if (result == enCodicoResult::success)
            if (value.state != toCpState(state))
            {
                value.state = toCpState(state);
                dbg.printf(enDebugLevel_Info, "Received from Codico get CP State: %s.", enCpState_toString(value.state).c_str());
            }
    } break;
    case CP_SUBID_NOTIFICATION_PWM:
    {
        uint16_t* pPwm = (uint16_t*)&pData[0];
        unsigned int pwm = (BigEndian(*pPwm) + 5) / 10;
        dbg.printf(enDebugLevel_Info, "Received from Codico CP Pwm Notification: PWM = %i%%.", (int)pwm);
        value.pwm = enCpPwm_decode(pwm);
    } break;
    case CP_SUBID_NOTIFICATION_STATE:
    {
        // see CP_SUBID_GetState
        int state = (int)pData[0];
        dbg.printf(enDebugLevel_Info, "Received from Codico CP State Notification: State = %s.", enCpState_toString(toCpState(state)).c_str());
        value.state = toCpState(state);
        if (initializationState != 4)
            dbg.printf(enDebugLevel_Debug, "Control Pilot initialized.");
        initializationState = 4;
    } break;
    case CP_SUBID_SetResistor:
    {
        enCodicoResult result = (enCodicoResult)pData[0];
        if (result == enCodicoResult::success)
        {
            receiveState = initializationState;
        }
        // enCodicoResult result = (enCodicoResult)*pData;
        // dbg.printf(enDebugLevel_Debug, "Received from Codico CP SetState: Result = %s.", enCodicoResult_toString(result).c_str());
    } break;
    default: dbg.printf(enDebugLevel_Debug, "Received from Codico CP an unimplemented message.");
    }
}

#pragma GCC diagnostic warning "-Wunused-parameter"

void cCodicoPilot::getCmdMode()
{
    dbg.printf(enDebugLevel_Debug, "Send getCmdMode.");
    sendFramingMessage(CP_SUBID_GetMode);
}

void cCodicoPilot::setCmdMode()
{
    // char mode0 = "Mode = EV";
    // char mode1 = "Mode = EVSE"

    uint8_t mode = (stackType == enStackType::secc) ? 1 : 0;
    dbg.printf(enDebugLevel_Debug, "Send setCmdMode %s.", mode ? "SECC" : "EVCC");
    sendFramingMessage(CP_SUBID_SetMode, &mode, 1);
}

void cCodicoPilot::getCmdPwm()
{
    dbg.printf(enDebugLevel_Debug, "Send getCmdPwm.");
    sendFramingMessage(CP_SUBID_GetPwm);
}

void cCodicoPilot::setCmdPwm(uint8_t pwm)
{
    uint16_t pwmValue = (uint16_t)pwm * 10;
    BigEndian(&pwmValue);
    dbg.printf(enDebugLevel_Info, "Send setCmdPwm %i%%.", (int)pwm);
    sendFramingMessage(CP_SUBID_SetPwm, (uint8_t*)&pwmValue, 2);
}

void cCodicoPilot::startService()
{
    dbg.printf(enDebugLevel_Debug, "Send startService.");
    sendFramingMessage(CP_SUBID_Start);
}

void cCodicoPilot::stopService()
{
    dbg.printf(enDebugLevel_Debug, "Send stopService.");
    sendFramingMessage(CP_SUBID_Stop);
}

void cCodicoPilot::getCmdState()
{
    //dbg.printf(enDebugLevel_Debug, "Send getCmdState.");
    sendFramingMessage(CP_SUBID_GetState);
}

void cCodicoPilot::setCmdState(uint8_t state)
{
//    0: State B,
//    1: State C,
//    2: State D
    //dbg.printf(enDebugLevel_Debug, "Send setCmdState %s.", enCpState_toString(toCpState(state)).c_str());
    sendFramingMessage(CP_SUBID_SetResistor, &state, 1);
}

/**
 * @brief Retrieves the actual state
 * @return
 */
stCpValue cCodicoPilot::get()
{
    return value;
}

/**
 * @brief Sets the desired state
 */
void cCodicoPilot::set(stCpValue value)
{
    this->value = value;
    if (stackType == enStackType::secc)
    {
        setCmdPwm((uint8_t)value.pwm);
    }
    else
    {
        setCmdState(fromCpState(value.state));
    }
}

/**
 * @brief Retrieves the last set desired state
 * @return
 */
stCpValue cCodicoPilot::getDemand()
{
    return value;
}

/**
 * @brief Retrieves the cable current capacity according to the PP-code
 * @return
 */
enPpCode cCodicoPilot::getPpCode()
{
    return enPpCode::UNKNOWN;
}

/**
 * @brief Synchronous state and communication processing
 */
void cCodicoPilot::operate()
{
    if (!isInitialized())
    {
        uint8_t zeros[ETHER_ADDR_LEN] = {0, 0, 0, 0, 0, 0};

        if (pPlc)
        {
            if (memcmp(pPlc->getQcaMAC(), zeros, ETHER_ADDR_LEN) != 0)
            {
                char addr[ETHER_ADDR_LEN];
                memcpy(addr, pPlc->getQcaMAC(), ETHER_ADDR_LEN);
                addr[5] -= 2;
                setAddress((uint8_t*)addr);
            }
        }

        if (memcmp(destinationAddress, zeros, ETHER_ADDR_LEN) != 0)
        {
            if (receiveState == initializationState)
            {
                switch (initializationState)
                {
                case 0:
                    setCmdMode();
                    break;
                case 1:
                    if (stackType == enStackType::secc)
                        setCmdPwm(100);
                    else
                        setState(enCpState::B_Connected);
                    break;
                case 2:
                    startService();
                    break;
                default:;
                }
                initializationState++;
            }
            // codico.getState();
        }
        if (isInitialized())
            dbg.printf(LibCpp::enDebugLevel_Info, "Codico control pilot is initialized.");
    }
    else
    {
//        if (pollTimer.elapsed())
//        {
//            pollTimer.start();
//            //dbg.printf(LibCpp::enDebugLevel_Debug, "Polling state.");
//            getCmdState();
//        }
    }
}

enCpState cCodicoPilot::toCpState(int state)
{
    // 12V:state A=0, 9V: state B=1, 6V: state C=2, 3V: state D=3, 0V: state E=4, -12V: state F=5, unknown=6.
    switch(state)
    {
    case 0: return enCpState::A_Open;
    case 1: return enCpState::B_Connected;
    case 2: return enCpState::C_Ready;
    case 3: return enCpState::D_ReadyVentilation;
    case 4: return enCpState::E_ShutDown;
    case 5: return enCpState::F_Error;
    default: return enCpState::Offline;
    }
}

int cCodicoPilot::fromCpState(enCpState state)
{
    //  Resistor setting states
    //    0: State B,
    //    1: State C,
    //    2: State D
    switch (state)
    {
    case enCpState::C_Ready: return 1;
    case enCpState::D_ReadyVentilation: return 2;
    default: return 0;
    }
}
