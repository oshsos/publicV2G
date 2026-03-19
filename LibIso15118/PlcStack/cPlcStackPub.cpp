#define EVSE_STATE_UNAVAILABLE 0
#define EVSE_STATE_UNOCCUPIED 1
#define EVSE_STATE_UNMATCHED 2
#define EVSE_STATE_MATCHED 3

#include <cstring>

#include "cPlcStackPub.h"
#include <string.h>

#include "../../LibCpp/HAL/Tools.h"

extern "C"
{
    #include "tools/ModleyTools.h"
}

#ifdef WIN32

#include <time.h>

#ifdef __GNUC__
int gettimeofday(timeval* tv, struct timezone* tz)
{
    return mingw_gettimeofday(tv, tz);
}
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#include <windows.h>
#include <sys/time.h>

int gettimeofday (struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;
    if (NULL != tv)
    {
        GetSystemTimeAsFileTime (&ft);
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
        tmpres /= 10;
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }
    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset ();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
    return 0;
}
#endif
#endif

#include "../../LibCpp/HAL/HW_Tools.h"
//#include "LibCpp/HAL/Tools.h"

using namespace std;
using namespace LibCpp;
using namespace Iso15118;

char MAC_ATHEROSPING[ETHER_ADDR_LEN] = {(char)0x00, (char)0xB0, (char)0x52, (char)0x00, (char)0x00, (char)0x01};

const char* StationRole [3] =
    {
        "STA",
        "PCO",
        "CCO"
};

//stSounding::stSounding()
//{
//    clear();
//}
//void stSounding::clear()
//{
//    ts.tv_sec = 0;
//    ts.tv_usec = 0;
//    tc.tv_sec = 0;
//    tc.tv_usec = 0;
//    timer = 0;
//    for (int i=0; i<SLAC_GROUPS; i++)
//        AAG[i] = 0;
//    sounds = 0;
//}

cPlcStackPub::cPlcStackPub(cPacketSocket* pSocket) :
    dbg("cPlcStackPub")
{
    sessionIdentNumber = 0;
    this->pSocket = pSocket;
    memset(qcaMAC, 0, ETHER_ADDR_LEN);

    clearSendBuffer();
    initializeSession();

    pSocket->setCallback(this);
}

bool cPlcStackPub::connectionSuccessful()
{
    return values.exit == 0;
}

void cPlcStackPub::clear()
{
    clearSendBuffer();
    initializeSession();
    // Initialize qca to start condition
    send_CM_SET_KEY_REQ();
}

void cPlcStackPub::initializeSession()
{
    memset(&values, 0, sizeof(session));

    hexencode (values.EVSE_ID, SLAC_UNIQUE_ID_LEN, EVSE_SID);
    hexencode (values.NMK, SLAC_NMK_LEN, EVSE_NMK);
    hexencode (values.NID, SLAC_NID_LEN, EVSE_NID);
    values.NUM_SOUNDS = SLAC_MSOUNDS;
    values.TIME_OUT = SLAC_TIMETOSOUND;
    values.RESP_TYPE = SLAC_RESPONSE_TYPE;
    values.chargetime = SLAC_CHARGETIME;
    values.settletime = SLAC_SETTLETIME;
    memcpy (values.original_nmk, values.NMK, SLAC_NMK_LEN);
    memcpy (values.original_nid, values.NID, SLAC_NID_LEN);
    values.state = EVSE_STATE_UNOCCUPIED;
    memcpy (values.EVSE_MAC, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN);
    values.sounds = 0;
    values.pause = SLAC_PAUSE;
    memset(values.RunID, 0, SLAC_RUNID_LEN);

    //sounding.clear();
    for (int i=0; i<SLAC_GROUPS; i++)
        values.AAG[i] = 0;
    values.sounds = 0;

    sessionIdentNumber++;
    paramRequestCount = 0;

}

bool cPlcStackPub::soundsComplete()
{
    cDebug dbg("soundsComplete", &this->dbg);
    if (values.sounds == values.NUM_SOUNDS)
    {
        dbg.printf(LibCpp::enDebugLevel_Debug, "Sounds are complete.");
        if (values.sounds > 0)
        {
            for (values.NumGroups = 0; values.NumGroups < SLAC_GROUPS; ++ values.NumGroups)
            {
                values.AAG[values.NumGroups] = values.AAG[values.NumGroups] / values.sounds;
            }
        }

        unsigned int sum = 0;
        int AAGaverage = 0;
        for (unsigned int i=0; i< values.NumGroups; i++)
            sum += values.AAG[i];
        if (values.NumGroups)
            AAGaverage = sum / values.NumGroups;
        dbg.printf(enDebugLevel_Info, "Attenuation 0x%02x [max: 0x%02x] (%s)", AAGaverage, SLAC_LIMIT, ByteArrayToString(values.AAG, values.NumGroups).c_str());

        return true;
    }
    return false;
}

bool cPlcStackPub::analyseSounding()
{
    cDebug dbg("analyseSounding",  &this->dbg);

    unsigned group = 0;
    unsigned total = 0;
    if (values.NumGroups > SLAC_GROUPS)
    {
        dbg.printf(LibCpp::enDebugLevel_Error, "Too much data to analyse!");
        return false;
    }
    if (values.NumGroups > 0)
    {
        while (group < values.NumGroups)
        {
            total += values.AAG [group];
            group++;
        }
        total /= group;
        if (total > values.limit)
        {
            dbg.printf(LibCpp::enDebugLevel_Error, "Average attenuation (%u) more than limit (%u) from %d groups!", total, values.limit, group);
            return false;
        }
        if (total > 0)
        {
            dbg.printf(LibCpp::enDebugLevel_Info, "Average attenuation is (%u) within the limit (%u) from %d groups!", total, values.limit, group);
            return true;
        }
    }
    dbg.printf(LibCpp::enDebugLevel_Error, "No data to analyse!");
    return false;
}

//void cPlcStackPub::initializeSounding()
//{
//}

void cPlcStackPub::clearSendBuffer()
{
    memset(&sendBuffer, 0, cPlcStack_BUFFERSIZE);
}

void cPlcStackPub::fillEthernetHdr(stEthernetHeader* pHeader, char destinationMAC[ETHER_ADDR_LEN])
{
    memcpy(pHeader->ODA, destinationMAC, ETHER_ADDR_LEN);   // Destination MAC
    memcpy(pHeader->OSA, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN); // Source MAC
    pHeader->MTYPE = BigEndian((uint16_t)ETH_MTYPE_HPAV);                        // Message type Homeplug AV
};

/**
 * @brief Fills the Homeplug AV header for Qualcomm manufacturer dependent messages.
 * @param pHeader
 * @param MMTYPE
 */
void cPlcStackPub::fillQualcommHdr(qualcomm_hdr* pHeader, uint16_t MMTYPE)
{
    pHeader->MMV = MME_MMV;                     // protocol version
    pHeader->MMTYPE = LittleEndian(MMTYPE);     // Manufacturer type (3 bit), message type (11 bit), Reqest/Response (2 bit)
    pHeader->OUI [0] = 0x00;                    // Qualcomm manufacturer ID
    pHeader->OUI [1] = 0xB0;
    pHeader->OUI [2] = 0x52;
}

/**
 * @brief Fills the standardized Homeplug AV header.
 * @param pHeader
 * @param MMTYPE
 */
void cPlcStackPub::fillHomePlugHdr(homeplug_fmi* pHeader, uint16_t MMTYPE)
{
    pHeader->MMV = HOMEPLUG_MMV;                // protocol version
    pHeader->MMTYPE = LittleEndian(MMTYPE);     // Manufacturer type (3 bit), message type (11 bit), Reqest/Response (2 bit)
    pHeader->FMSN = 0x00;
    pHeader->FMID = 0x00;
}

////////////////// QCA Chip message sending ////////////////

void cPlcStackPub::send_GET_SW_VER_REQ()
{
    cDebug dbg("send_GET_SW_VER_REQ", &this->dbg);
    vs_sw_ver_request* pMsg = (vs_sw_ver_request*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, MAC_ATHEROSPING);
    fillQualcommHdr(&pMsg->qualcomm, LittleEndian((uint16_t)(VS_SW_VER | MMTYPE_REQ)));
    pMsg->COOKIE = BigEndian((uint32_t)VAL_COOKIE);
    dbg.printf(enDebugLevel_Debug, "Sent: GET_SW_VER.REQ .");
    pSocket->send(sendBuffer, 60);
}

void cPlcStackPub::send_CM_SET_KEY_REQ()
{
    cDebug dbg("send_CM_SET_KEY_REQ", &this->dbg);
    cm_set_key_request* pMsg = (cm_set_key_request*)sendBuffer;
    clearSendBuffer();
    // fillEthernetHdr(&pMsg->ethernet, (char*)values.PEV_MAC);
    fillEthernetHdr(&pMsg->ethernet, (char*)qcaMAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_SET_KEY | MMTYPE_REQ));
    pMsg->KEYTYPE = SLAC_CM_SETKEY_KEYTYPE;
    memset(&pMsg->MYNOUNCE, 0xAA, sizeof(uint32_t));
    memset(&pMsg->YOURNOUNCE, 0x00, sizeof(uint32_t));
    pMsg->PID = SLAC_CM_SETKEY_PID;
    pMsg->PRN = LittleEndian((uint16_t)SLAC_CM_SETKEY_PRN);
    pMsg->PMN = SLAC_CM_SETKEY_PMN;
    pMsg->CCOCAP = SLAC_CM_SETKEY_CCO;
    memcpy (pMsg->NID, values.NID, SLAC_NID_LEN);
    pMsg->NEWEKS = SLAC_CM_SETKEY_EKS;
    memcpy (pMsg->NEWKEY, values.NMK, SLAC_NMK_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent send_CM_SET_KEY.REQ . NID = %s, NMK = %s", ByteArrayToString((char*)pMsg->NID, SLAC_NID_LEN).c_str(), ByteArrayToString((char*)pMsg->NEWKEY, SLAC_NMK_LEN).c_str());
    pSocket->send(sendBuffer, sizeof(cm_atten_char_indicate));
}

void cPlcStackPub::send_VS_PL_LINK_STATUS_REQ()
{
    cDebug dbg("send_VS_PL_LINK_STATUS_REQ", &this->dbg);
    vs_link_status_request* pMsg = (vs_link_status_request*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)qcaMAC);
    fillQualcommHdr(&pMsg->qualcomm, LittleEndian((uint16_t)(VS_PL_LINK_STATUS | MMTYPE_REQ)));
    dbg.printf(enDebugLevel_Debug, "Sent: VS_PL_LINK_STATUS.REQ .");
    pSocket->send(sendBuffer, 60);
}

void cPlcStackPub::send_VS_NW_INFO_REQ()
{
    cDebug dbg("send_VS_PL_LINK_STATUS_REQ", &this->dbg);
    vs_link_status_request* pMsg = (vs_link_status_request*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)qcaMAC);
    fillQualcommHdr(&pMsg->qualcomm, LittleEndian((uint16_t)(VS_NW_INFO | MMTYPE_REQ)));
    dbg.printf(enDebugLevel_Debug, "Sent: send_VS_NW_INFO.REQ .");
    pSocket->send(sendBuffer, 60);
}

////////////////// SECC homeplug AV message sending ////////////////

void cPlcStackPub::send_CM_SLAC_PARAM_CNF()
{
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    cDebug dbg("send_CM_SLAC_PARAM_CNF", &this->dbg);
    cm_slac_param_confirm* pMsg = (cm_slac_param_confirm*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.PEV_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_SLAC_PARAM | MMTYPE_CNF));
    memcpy(pMsg->MSOUND_TARGET, broadcastMac, ETHER_ADDR_LEN);
    pMsg->NUM_SOUNDS = values.NUM_SOUNDS;
    pMsg->TIME_OUT = values.TIME_OUT;
    pMsg->RESP_TYPE = values.RESP_TYPE;
    memcpy (pMsg->FORWARDING_STA, values.FORWARDING_STA, ETHER_ADDR_LEN);
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    pMsg->CipherSuite = LittleEndian((uint16_t)values.counter);  // Might be some kind of misuse?
    memcpy(pMsg->RunID, values.RunID, SLAC_RUNID_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_SLAC_PARAM.CNF .");
    pSocket->send(sendBuffer, 60);
}

void cPlcStackPub::send_CM_ATTEN_CHAR_IND()
{
    cDebug dbg("send_CM_ATTEN_CHAR_IND", &this->dbg);
//    if (values.sounds > 0)
//    {
//        for (values.NumGroups = 0; values.NumGroups < SLAC_GROUPS; ++ values.NumGroups)
//        {
//            values.AAG[values.NumGroups] = values.AAG[values.NumGroups] / values.sounds;
//        }
//        //values.sounds = sounding.sounds;
//    }
    cm_atten_char_indicate* pMsg = (cm_atten_char_indicate*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.PEV_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_ATTEN_CHAR | MMTYPE_IND));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    memcpy (pMsg->ACVarField.SOURCE_ADDRESS, values.PEV_MAC, ETHER_ADDR_LEN);
    memcpy (pMsg->ACVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    memset (pMsg->ACVarField.SOURCE_ID, 0, SLAC_UNIQUE_ID_LEN);
    memset (pMsg->ACVarField.RESP_ID, 0, SLAC_UNIQUE_ID_LEN);
    pMsg->ACVarField.NUM_SOUNDS = values.sounds;
    pMsg->ACVarField.ATTEN_PROFILE.NumGroups = values.NumGroups;
    memcpy (pMsg->ACVarField.ATTEN_PROFILE.AAG, values.AAG, values.NumGroups);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_ATTEN_CHAR.IND .");
    pSocket->send(sendBuffer, sizeof(cm_atten_char_indicate));
}

void cPlcStackPub::send_CM_SLAC_MATCH_CNF()
{
    cDebug dbg("send_CM_SLAC_MATCH_CNF", &this->dbg);
    cm_slac_match_confirm* pMsg = (cm_slac_match_confirm*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.PEV_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_SLAC_MATCH | MMTYPE_CNF));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    pMsg->MVFLength = LittleEndian((uint16_t)sizeof(pMsg->MatchVarField));
    memcpy (pMsg->MatchVarField.PEV_ID, values.PEV_ID, SLAC_UNIQUE_ID_LEN);
    memcpy (pMsg->MatchVarField.PEV_MAC, values.PEV_MAC, ETHER_ADDR_LEN);
    memcpy (pMsg->MatchVarField.EVSE_ID, values.EVSE_ID, SLAC_UNIQUE_ID_LEN);
    memcpy (pMsg->MatchVarField.EVSE_MAC, values.EVSE_MAC, ETHER_ADDR_LEN);
    memcpy (pMsg->MatchVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    memcpy (pMsg->MatchVarField.NID, values.NID, SLAC_NID_LEN);
    memcpy (pMsg->MatchVarField.NMK, values.NMK, SLAC_NMK_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_SLAC_MATCH.CNF .");
    pSocket->send(sendBuffer, sizeof(cm_slac_match_confirm));
    //analyseSounding();
}

void cPlcStackPub::send_VS_PEER_PING_REQ()
{
    cDebug dbg("send_VS_PEER_PING_REQ", &this->dbg);
    vs_peer_ping* pMsg = (vs_peer_ping*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)MAC_BROADCAST);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(VS_PEER_PING | MMTYPE_REQ));
    dbg.printf(enDebugLevel_Debug, "Sent: VS_PEER_PING.REQ .");
    pSocket->send(sendBuffer, 60);
}

void cPlcStackPub::send_VS_PEER_PING_RSP()
{
    cDebug dbg("send_VS_PEER_PING_RSP", &this->dbg);
    vs_peer_ping* pMsg = (vs_peer_ping*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.PEV_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(VS_PEER_PING | MMTYPE_RSP));
    dbg.printf(enDebugLevel_Debug, "Sent: VS_PEER_PING.RSP .");
    pSocket->send(sendBuffer, 60);

}

////////////////// EVCC homeplug AV message sending ////////////////

void cPlcStackPub::send_CM_SLAC_PARAM_REQ()
{
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    cDebug dbg("send_CM_SLAC_PARAM_REQ", &this->dbg);
    cm_slac_param_request* pMsg = (cm_slac_param_request*)sendBuffer;
    clearSendBuffer();
    // fillEthernetHdr(&pMsg->ethernet, qcaMAC);
    fillEthernetHdr(&pMsg->ethernet, (char*)broadcastMac);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_SLAC_PARAM | MMTYPE_REQ));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    pMsg->CipherSuite[0] = LittleEndian((uint16_t)values.counter);                          // Might be some kind of misuse?
    memcpy(values.RunID, qcaMAC, ETHER_ADDR_LEN);                                           // Just to have some kind of unique ID (unique session of different hardwares / charging sockets)
    short sessionID = (short)sessionIdentNumber;
    BigEndian(&sessionID);
    memcpy(values.RunID + ETHER_ADDR_LEN, &sessionID, SLAC_RUNID_LEN - ETHER_ADDR_LEN);     // Just to have some kind of unique ID (unique session on same hardware)
    memcpy(pMsg->RunID, values.RunID, SLAC_RUNID_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_SLAC_PARAM.REQ .");
    pSocket->send(sendBuffer, 60);
}

void cPlcStackPub::send_CM_START_ATTEN_CHAR_IND()
{
    cDebug dbg("send_CM_START_ATTEN_CHAR_IND", &this->dbg);
    cm_start_atten_char_indicate* pMsg = (cm_start_atten_char_indicate*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values. MSOUND_TARGET);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_START_ATTEN_CHAR | MMTYPE_IND));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    memcpy (pMsg->ACVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    pMsg->ACVarField.NUM_SOUNDS = values.NUM_SOUNDS;
    pMsg->ACVarField.TIME_OUT = values.TIME_OUT;
    pMsg->ACVarField.RESP_TYPE = values.RESP_TYPE;
    memcpy (pMsg->ACVarField.FORWARDING_STA, values.FORWARDING_STA, ETHER_ADDR_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_START_ATTEN_CHAR.IND .");
    pSocket->send(sendBuffer, 60);
}

// ???? Statt CNF wird hier IND gesendet!
//void cPlcStackPub::send_CM_START_ATTEN_CHAR_CNF()
//{
//    cDebug dbg("send_CM_START_ATTEN_CHAR_CNF", &this->dbg);
//    initializeSounding();
//    cm_start_atten_char_indicate* pMsg = (cm_start_atten_char_indicate*)sendBuffer;
//    clearSendBuffer();
//    fillEthernetHdr(&pMsg->ethernet, (char*)values. MSOUND_TARGET);
//    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_START_ATTEN_CHAR | MMTYPE_CNF));
//    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
//    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
//    pMsg->ACVarField.NUM_SOUNDS = values.NUM_SOUNDS;
//    pMsg->ACVarField.TIME_OUT = values.TIME_OUT;
//    pMsg->ACVarField.RESP_TYPE = values.RESP_TYPE;
//    memcpy (pMsg->ACVarField.FORWARDING_STA, values.FORWARDING_STA, ETHER_ADDR_LEN);
//    memcpy (pMsg->ACVarField.RunID, values.RunID, SLAC_RUNID_LEN);
//    dbg.printf(enDebugLevel_Debug, "Sent: CM_START_ATTEN_CHAR.CNF .");
//    pSocket->send(sendBuffer, 60);
//}

void cPlcStackPub::send_CM_MNBC_SOUND_IND(int soundNumDecrement)
{
    cDebug dbg("send_CM_MNBC_SOUND_IND", &this->dbg);
    cm_mnbc_sound_indicate* pMsg = (cm_mnbc_sound_indicate*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values. MSOUND_TARGET);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_MNBC_SOUND | MMTYPE_IND));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    memcpy (pMsg->MSVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    memcpy (pMsg->MSVarField.SenderID, values.PEV_ID, SLAC_UNIQUE_ID_LEN);
    pMsg->MSVarField.CNT = soundNumDecrement;
    memset (pMsg->MSVarField.RND, 0, SLAC_UNIQUE_ID_LEN);
    //memcpy (pMsg->MSVarField.RND, values.RND, SLAC_UNIQUE_ID_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_MNBC_SOUND.IND Nr.:%i.", soundNumDecrement);
    pSocket->send(sendBuffer, sizeof(cm_mnbc_sound_indicate));
}

void cPlcStackPub::send_CM_ATTEN_CHAR_RSP()
{
    cDebug dbg("send_CM_ATTEN_CHAR_RSP", &this->dbg);
    cm_atten_char_response* pMsg = (cm_atten_char_response*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.EVSE_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_ATTEN_CHAR | MMTYPE_RSP));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    memcpy (pMsg->ACVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    memcpy (pMsg->ACVarField.SOURCE_ADDRESS, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN);
    //memcpy (pMsg->ACVarField.SOURCE_ADDRESS, values.PEV_MAC, ETHER_ADDR_LEN);
    memset (pMsg->ACVarField.SOURCE_ID, 0, SLAC_UNIQUE_ID_LEN);
    memset (pMsg->ACVarField.RESP_ID, 0, SLAC_UNIQUE_ID_LEN);
    pMsg->ACVarField.Result = 0;
    dbg.printf(enDebugLevel_Debug, "Sent: CM_ATTEN_CHAR.RSP .");
    pSocket->send(sendBuffer, sizeof(cm_atten_char_response));
}

void cPlcStackPub::send_CM_SLAC_MATCH_REQ()
{
    cDebug dbg("send_CM_SLAC_MATCH_REQ", &this->dbg);
    cm_slac_match_request* pMsg = (cm_slac_match_request*)sendBuffer;
    clearSendBuffer();
    fillEthernetHdr(&pMsg->ethernet, (char*)values.EVSE_MAC);
    fillHomePlugHdr(&pMsg->homeplug, (uint16_t)(CM_SLAC_MATCH | MMTYPE_REQ));
    pMsg->APPLICATION_TYPE = values.APPLICATION_TYPE;
    pMsg->SECURITY_TYPE = values.SECURITY_TYPE;
    memcpy (pMsg->MatchVarField.RunID, values.RunID, SLAC_RUNID_LEN);
    pMsg->MVFLength = LittleEndian((uint16_t)sizeof(pMsg->MatchVarField));
    memcpy (pMsg->MatchVarField.PEV_ID, values.PEV_ID, SLAC_UNIQUE_ID_LEN);
    //memcpy (pMsg->MatchVarField.PEV_MAC, values.PEV_MAC, ETHER_ADDR_LEN);
    memcpy (pMsg->MatchVarField.PEV_MAC, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN);
    memcpy (pMsg->MatchVarField.EVSE_MAC, values.EVSE_MAC, ETHER_ADDR_LEN);
    // memcpy (pMsg->MatchVarField.EVSE_ID, values.EVSE_ID, SLAC_UNIQUE_ID_LEN);
    dbg.printf(enDebugLevel_Debug, "Sent: CM_SLAC_MATCH.REQ .");
    pSocket->send(sendBuffer, sizeof(cm_slac_match_request));
    //analyseSounding();
}

bool cPlcStackPub::checkMessage(string messageType, uint8_t mmv, uint8_t appType, uint8_t secType, uint8_t* runId)
{
    bool ok = true;
    string errString = "Received " + messageType + " message.";
    if (mmv != HOMEPLUG_MMV)
    {
        ok = false;
        errString += stringFormat(" Unsupportet version %i! Required version is %i!", (int)mmv, (int)HOMEPLUG_MMV);
    }
    if (appType != values.APPLICATION_TYPE)
    {
        ok = false;
        errString += stringFormat(" Unsupportet version %i! Required version is %i!", (int)appType, (int)values.APPLICATION_TYPE);
    }
    if (secType != values.SECURITY_TYPE)
    {
        ok = false;
        errString += stringFormat(" Unsupportet version %i! Required version is %i!", (int)secType, (int)values.SECURITY_TYPE);
    }
    if (runId)
    {
        if (memcmp(runId, values.RunID, SLAC_RUNID_LEN) != 0)
        {
            ok = false;
            errString += stringFormat(" False RunID %s! Expected RunID is %s!", ByteArrayToString(runId, SLAC_RUNID_LEN).c_str(), ByteArrayToString(values.RunID, SLAC_RUNID_LEN).c_str());
        }
    }
    if (ok)
        dbg.printf(enDebugLevel_Debug, "Received %s .", messageType.c_str());
    else
        dbg.printf(enDebugLevel_Error, errString.c_str());
    return ok;
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
bool cPlcStackPub::onReceiveFrameCheck(cFramePort* pFramePort, char* receiveBuffer, unsigned int* pLen, unsigned int bufferSize)
{
    stEthernetHeader* pHdr = (stEthernetHeader*)receiveBuffer;
    if (pHdr->MTYPE == BigEndian((uint16_t)ETH_MTYPE_HPAV))
        return true;
    return false;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

void cPlcStackPub::onFrameReceive(cFramePort* pFramePort)
{
    cDebug dbg("onFrameReceive", &this->dbg);

    // dbg.printf(LibCpp::enDebugLevel_Debug, "Callback receive.");

    char* receiveMessage = nullptr;
    int receiveMessageLen = pFramePort->receiveBuffer(&receiveMessage);

    if (receiveMessageLen)
    {
        stEthernetHeader* pHdr = (stEthernetHeader*)receiveMessage;
        if (pHdr->MTYPE == BigEndian((uint16_t)ETH_MTYPE_HPAV))
            evaluateMessage(receiveMessage, receiveMessageLen);
    }

    pFramePort->receiveAcknowledge();
    return;
}

uint16_t cPlcStackPub::evaluateMessage(char* receiveMessage, int receiveMessageLen)
{
    cDebug dbg("evaluateMessage", &this->dbg);

    if (receiveMessageLen)
    {
        // dbg.printf(enDebugLevel_Debug, "Received %i bytes: %s", receiveMessageLen, ByteArrayToString(receiveMessage, receiveMessageLen).c_str());        
        stEthernetMsg* pEtherMsg = (stEthernetMsg*)receiveMessage;
        if (pEtherMsg->header.MTYPE == BigEndian((uint16_t)ETH_MTYPE_HPAV))
        {   // valid Homeplug AV message
            homeplug_hdr* pHomePlugHdr = (homeplug_hdr*)&pEtherMsg->payload;

            ////////////////// QCA chip to host message reception ////////////////

            if (pHomePlugHdr->MMTYPE == (uint16_t)(VS_SW_VER | MMTYPE_CNF))
            {   // GET_SW_VER.CNF
                vs_sw_ver_confirm* pMsg = (vs_sw_ver_confirm*)receiveMessage;
                if (pMsg->qualcomm.MMV != MME_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received GET_SW_VER.CNF message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)MME_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                memcpy(qcaMAC, pMsg->ethernet.OSA, ETHER_ADDR_LEN);
                int len = pMsg->MVERLENGTH;
                if (len>254-1) len = 254-1;
                memcpy(values.deviceVersion, pMsg->MVERSION, len);
                values.deviceVersion[len] = 0;
                dbg.printf(enDebugLevel_Info, "Received GET_SW_VER.CNF: %s from MAC address %s .", values.deviceVersion, cPacketSocket::mac_toString(qcaMAC).c_str());
                lastEvaluationResult = VS_SW_VER;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_SET_KEY | MMTYPE_CNF))
            {   // CM_SET_KEY.CNF
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_SET_KEY.CNF message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_set_key_confirm* pMsg = (cm_set_key_confirm*)receiveMessage;
                if (pMsg->RESULT)
                    values.exit = 0;
                else
                    values.exit = 1;
                dbg.printf(enDebugLevel_Debug, "Received CM_SET_KEY.CNF: %s", values.exit ? "Refused" : "Success");
                lastEvaluationResult = CM_SET_KEY;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(VS_PL_LINK_STATUS | MMTYPE_CNF))
            {   // VS_PL_LINK_STATUS.CNF
                // Version of this message is zero, thus no version check is executed at this point.
                vs_link_status_confirm* pMsg = (vs_link_status_confirm*)receiveMessage;
                values.linkStatus = pMsg->LINKSTATUS;
                dbg.printf(enDebugLevel_Debug, "Received VS_PL_LINK_STATUS.CNF: %s", values.linkStatus ? "Online" : "Offline");
                lastEvaluationResult = VS_PL_LINK_STATUS;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(VS_PEER_PING | MMTYPE_REQ))
            {   // VS_PEER_PING.REQ
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received VS_PEER_PING.REQ message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                vs_peer_ping* pMsg = (vs_peer_ping*)receiveMessage;
                dbg.printf(enDebugLevel_Debug, "Received VS_PEER_PING.REQ from peer %s .", ByteArrayToString(pMsg->ethernet.OSA, ETHER_ADDR_LEN).c_str());
                send_VS_PEER_PING_RSP();
                lastEvaluationResult = VS_PEER_PING;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(VS_PEER_PING | MMTYPE_RSP))
            {   // VS_PEER_PING.RSP
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received VS_PEER_PING.RSP message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                vs_peer_ping* pMsg = (vs_peer_ping*)receiveMessage;
                dbg.printf(enDebugLevel_Info, "Received VS_PEER_PING.RSP from peer %s .", ByteArrayToString(pMsg->ethernet.OSA, ETHER_ADDR_LEN).c_str());
                lastEvaluationResult = VS_PEER_PING;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(VS_NW_INFO | MMTYPE_CNF))
            {   // VS_NW_INFO.CNF
                // Version of this message is zero, thus no version check is executed at this point.
                vs_nw_info_confirm* pMsg = (vs_nw_info_confirm*)receiveMessage;
                dbg.printf(enDebugLevel_Debug, "Received VS_NW_INFO.CNF .");

//                struct networks *networks = (struct networks *) (pMsg->DATA);
                struct networks *networks = (struct networks *) (pMsg->v0.content);
                string out;
                out += stringFormat("\nFound %d Network(s)\n", networks->v0.NUMAVLNS);
                struct network *network = (struct network *)(&networks->v0.networks);
                memset(values.CCO_MAC, 0, ETHER_ADDR_LEN);
                memset(values.STA_MAC, 0, ETHER_ADDR_LEN);
                while (networks->v0.NUMAVLNS--)
                {
                    out += stringFormat("\tnetwork->NID = %s\n", ByteArrayToString(network->v0.NID, SLAC_NID_LEN).c_str());
                    out += stringFormat("\tnetwork->SNID = %d\n", network->v0.SNID);
                    out += stringFormat("\tnetwork->TEI = %d\n", network->v0.TEI);
                    out += stringFormat("\tnetwork->ROLE = 0x%02X (%s)\n", network->v0.ROLE, StationRole [network->v0.ROLE]);
                    out += stringFormat("\tnetwork->CCO_DA = %s\n", ByteArrayToString(network->v0.CCO_MAC, ETHER_ADDR_LEN).c_str());
                    out += stringFormat("\tnetwork->CCO_TEI = %d\n", network->v0.CCO_TEI);
                    out += stringFormat("\tnetwork->STATIONS = %d\n", network->v0.NUMSTAS);
                    out += "\n";
                    struct station *station = (struct station *)(&network->v0.stations);
                    bool first = true;
                    while (network->v0.NUMSTAS--)
                    {
                        if (memcmp(network->v0.NID, values.NID, SLAC_NID_LEN)==0)
                        {
                            memcpy(values.CCO_MAC, network->v0.CCO_MAC, ETHER_ADDR_LEN);
                            if (first)
                            {
                                memcpy(values.STA_MAC, station->v0.MAC, ETHER_ADDR_LEN);
                                first = false;
                            }
                        }
                        out += stringFormat("\t\tstation->MAC = %s\n", ByteArrayToString(station->v0.MAC, ETHER_ADDR_LEN).c_str());
                        out += stringFormat("\t\tstation->TEI = %d\n", station->v0.TEI);
                        out += stringFormat("\t\tstation->BDA = %s\n", ByteArrayToString(station->v0.BDA, ETHER_ADDR_LEN).c_str());
                        station->v0.AVGTX = LittleEndian(station->v0.AVGTX);
                        station->v0.AVGRX = LittleEndian(station->v0.AVGRX);
                        out += stringFormat("\t\tstation->AvgPHYDR_TX = %03d mbps\n", station->v0.AVGTX);
                        out += stringFormat("\t\tstation->AvgPHYDR_RX = %03d mbps\n", station->v0.AVGRX);
                        out += stringFormat("\n");
                        station++;
                    }
                    network = (struct network *)(station);
                }
                out += "Expected communication partners:\n";
                out += stringFormat("CCO MAC = %s\n", ByteArrayToString(values.CCO_MAC, ETHER_ADDR_LEN).c_str());
                out += stringFormat("STA MAC = %s\n", ByteArrayToString(values.STA_MAC, ETHER_ADDR_LEN).c_str());
                out += "------";

                dbg.printf(enDebugLevel_Info, "\n%s", out.c_str());

                lastEvaluationResult = VS_NW_INFO;
                return lastEvaluationResult;
            }

            ////////////////// SECC message reception ////////////////

            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_SLAC_PARAM | MMTYPE_REQ))
            {   // CM_SLAC_PARAM.REQ
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_SLAC_PARAM.REQ message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_slac_param_request* pMsg = (cm_slac_param_request*)receiveMessage;
                values.APPLICATION_TYPE = pMsg->APPLICATION_TYPE;
                values.SECURITY_TYPE = pMsg->SECURITY_TYPE;
                memcpy(values.PEV_MAC, pMsg->ethernet.OSA, ETHER_ADDR_LEN);
                memcpy (values.FORWARDING_STA, pMsg->ethernet.OSA, ETHER_ADDR_LEN);
                memcpy (values.RunID, pMsg->RunID, SLAC_RUNID_LEN);
                dbg.printf(enDebugLevel_Debug, "Received CM_SLAC_PARAM.REQ .");
                lastEvaluationResult = CM_SLAC_PARAM;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_START_ATTEN_CHAR | MMTYPE_IND))
            {   // CM_START_ATTEN_CHAR.IND
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_START_ATTEN_CHAR.IND message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_start_atten_char_indicate* pMsg = (cm_start_atten_char_indicate*)receiveMessage;
                if (memcmp (values.RunID, pMsg->ACVarField.RunID, SLAC_RUNID_LEN) != 0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.IND . Unregistered RunID!");
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                values.APPLICATION_TYPE = pMsg->APPLICATION_TYPE;
                values.SECURITY_TYPE = pMsg->SECURITY_TYPE;
                values.NUM_SOUNDS = pMsg->ACVarField.NUM_SOUNDS;
                values.TIME_OUT = pMsg->ACVarField.TIME_OUT;
                if (pMsg->ACVarField.RESP_TYPE != values.RESP_TYPE)
                    dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.IND . Unexpected RESP_TYPE value %i instead of %i!", pMsg->ACVarField.RESP_TYPE, values.RESP_TYPE);
                memcpy (values.FORWARDING_STA, pMsg->ethernet.OSA, ETHER_ADDR_LEN);

                dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.IND . Number of sounds = %i .", values.NUM_SOUNDS);
                lastEvaluationResult = CM_START_ATTEN_CHAR;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_MNBC_SOUND | MMTYPE_IND))
            {   // CM_MNBC_SOUND.IND
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_MNBC_SOUND.IND message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_mnbc_sound_indicate* pMsg = (cm_mnbc_sound_indicate*)receiveMessage;
                if (memcmp (values.RunID, pMsg->MSVarField.RunID, SLAC_RUNID_LEN) != 0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_MNBC_SOUND.IND . Unregistered RunID!");
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                if (memcmp (values.PEV_MAC, pMsg->ethernet.OSA, ETHER_ADDR_LEN)!=0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_MNBC_SOUND.IND . Unexpected source MAC address!");
                }
                // values.sounds++;
                dbg.printf(enDebugLevel_Debug, "Received CM_MNBC_SOUND.IND . Remaining : %i", pMsg->MSVarField.CNT);
                lastEvaluationResult = CM_MNBC_SOUND;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_ATTEN_PROFILE | MMTYPE_IND))
            {   // CM_ATTEN_PROFILE.IND
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_ATTEN_PROFILE.IND message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_atten_profile_indicate* pMsg = (cm_atten_profile_indicate*)receiveMessage;
                if (memcmp (values.PEV_MAC, pMsg->PEV_MAC, ETHER_ADDR_LEN)!=0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_ATTEN_PROFILE.IND . Unexpected EV MAC address!");
                }
                for (values.NumGroups = 0; values.NumGroups < pMsg->NumGroups; values.NumGroups++)
                {
                    values.AAG[values.NumGroups] += pMsg->AAG[values.NumGroups];
                }
                values.NumGroups = pMsg->NumGroups;
                values.sounds++;
                dbg.printf(enDebugLevel_Debug, "Received CM_ATTEN_PROFILE.IND . Count = %i.", (int)values.sounds);
                lastEvaluationResult = CM_ATTEN_PROFILE;
                return lastEvaluationResult;
            }
            //            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_START_ATTEN_CHAR | MMTYPE_CNF))
            //            {   // CM_START_ATTEN_CHAR.CNF
            //                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
            //                {
            //                    dbg.printf(enDebugLevel_Fatal, "Received CM_START_ATTEN_CHAR.CNF message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
            //                    lastEvaluationResult = 0xFFFE;
            //                    return lastEvaluationResult;
            //                }
            //                cm_start_atten_char_indicate* pMsg = (cm_start_atten_char_indicate*)receiveMessage;
            //                if (memcmp (values.RunID, pMsg->ACVarField.RunID, SLAC_RUNID_LEN) != 0)
            //                {
            //                    dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.CNF. Unregistered RunID!");
            //                    lastEvaluationResult = 0xFFFE;
            //                    return lastEvaluationResult;
            //                }
            //                values.APPLICATION_TYPE = pMsg->APPLICATION_TYPE;
            //                values.SECURITY_TYPE = pMsg->SECURITY_TYPE;
            //                values.NUM_SOUNDS = pMsg->ACVarField.NUM_SOUNDS;
            //                values.TIME_OUT = pMsg->ACVarField.TIME_OUT;
            //                if (pMsg->ACVarField.RESP_TYPE != values.RESP_TYPE)
            //                    dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.CNF. Unexpected RESP_TYPE value %i instead of %i!", pMsg->ACVarField.RESP_TYPE, values.RESP_TYPE);
            //                memcpy (values.FORWARDING_STA, pMsg->ethernet.OSA, ETHER_ADDR_LEN);

            //                dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.CNF.");
            //                lastEvaluationResult = CM_START_ATTEN_CHAR;
            //                return lastEvaluationResult;
            //            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_START_ATTEN_CHAR | MMTYPE_IND))
            {   // CM_START_ATTEN_CHAR.IND
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_START_ATTEN_CHAR.IND message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_atten_char_indicate* pMsg = (cm_atten_char_indicate*)receiveMessage;
                if (memcmp (values.RunID, pMsg->ACVarField.RunID, SLAC_RUNID_LEN) != 0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.IND . Unregistered RunID!");
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                memcpy (values.EVSE_MAC, pMsg->ethernet.OSA, ETHER_ADDR_LEN);
                values.NUM_SOUNDS = pMsg->ACVarField.NUM_SOUNDS;
                values.NumGroups = pMsg->ACVarField.ATTEN_PROFILE.NumGroups;
                memcpy (values.AAG, pMsg->ACVarField.ATTEN_PROFILE.AAG, ETHER_ADDR_LEN);
                dbg.printf(enDebugLevel_Debug, "Received CM_START_ATTEN_CHAR.IND . Count = %i.", (int)values.sounds);
                lastEvaluationResult = CM_START_ATTEN_CHAR;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_ATTEN_CHAR | MMTYPE_RSP))
            {   // CM_ATTEN_CHAR.RSP
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_ATTEN_CHAR.RSP message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_atten_char_response* pMsg = (cm_atten_char_response*)receiveMessage;
                if (memcmp (values.RunID, pMsg->ACVarField.RunID, SLAC_RUNID_LEN) != 0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_ATTEN_CHAR.RSP message with unexpected RunID!");
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                dbg.printf(enDebugLevel_Debug, "Received CM_ATTEN_CHAR.RSP .");
                lastEvaluationResult = CM_ATTEN_CHAR;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_SLAC_MATCH | MMTYPE_REQ))
            {   // CM_SLAC_MATCH.REQ
                if (pHomePlugHdr->MMV != HOMEPLUG_MMV)
                {
                    dbg.printf(enDebugLevel_Fatal, "Received CM_SLAC_MATCH.REQ message version %i is not supported. Required version is %i!", (int)pHomePlugHdr->MMV, (int)HOMEPLUG_MMV);
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                cm_slac_match_request* pMsg = (cm_slac_match_request*)receiveMessage;
                if (memcmp (values.RunID, pMsg->MatchVarField.RunID, SLAC_RUNID_LEN) != 0)
                {
                    dbg.printf(enDebugLevel_Debug, "Received CM_SLAC_MATCH.REQ message with unexpected RunID!");
                    lastEvaluationResult = 0xFFFE;
                    return lastEvaluationResult;
                }
                memcpy (values.PEV_ID, pMsg->MatchVarField.PEV_ID, SLAC_UNIQUE_ID_LEN);
                memcpy (values.PEV_MAC, pMsg->MatchVarField.PEV_MAC, ETHER_ADDR_LEN);
                memcpy (values.RunID, pMsg->MatchVarField.RunID, SLAC_RUNID_LEN);
                dbg.printf(enDebugLevel_Debug, "Received CM_SLAC_MATCH.REQ .");
                lastEvaluationResult = CM_SLAC_MATCH;
                return lastEvaluationResult;
            }

            ////////////////// EVCC message reception ////////////////

            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_SLAC_PARAM | MMTYPE_CNF))
            {   // CM_SLAC_PARAM.CNF
                cm_slac_param_confirm* pMsg = (cm_slac_param_confirm*)receiveMessage;
                if (!checkMessage("CM_SLAC_PARAM.CNF", pHomePlugHdr->MMV, pMsg->APPLICATION_TYPE, pMsg->SECURITY_TYPE, 0))
                {
                    lastEvaluationResult = CM_SLAC_INVALID;
                    return CM_SLAC_INVALID;
                }
                memcpy(values.EVSE_MAC, pMsg->ethernet.OSA, ETHER_ADDR_LEN);
                memcpy (values.FORWARDING_STA, pMsg->FORWARDING_STA, ETHER_ADDR_LEN);
                memcpy (values.MSOUND_TARGET, pMsg->MSOUND_TARGET, ETHER_ADDR_LEN);
                values.NUM_SOUNDS = pMsg->NUM_SOUNDS;
                values.TIME_OUT = pMsg->TIME_OUT;
                values.RESP_TYPE = pMsg->RESP_TYPE;
                dbg.printf(enDebugLevel_Debug, "Received CM_SLAC_PARAM.CNF .");
                lastEvaluationResult = CM_SLAC_PARAM;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_ATTEN_CHAR | MMTYPE_IND))
            {   // CM_ATTEN_CHAR.IND
                cm_atten_char_indicate* pMsg = (cm_atten_char_indicate*)receiveMessage;
                if (!checkMessage("CM_ATTEN_CHAR.IND", pHomePlugHdr->MMV, pMsg->APPLICATION_TYPE, pMsg->SECURITY_TYPE, pMsg->ACVarField.RunID))
                {
                    lastEvaluationResult = CM_SLAC_INVALID;
                    return CM_SLAC_INVALID;
                }
                values.NUM_SOUNDS = pMsg->ACVarField.NUM_SOUNDS;
                values.NumGroups = pMsg->ACVarField.ATTEN_PROFILE.NumGroups;
                if (values.NumGroups > SLAC_GROUPS) values.NumGroups = SLAC_GROUPS;
                memcpy(values.AAG, pMsg->ACVarField.ATTEN_PROFILE.AAG, values.NumGroups);
                dbg.printf(enDebugLevel_Debug, "Received CM_ATTEN_CHAR.IND .");
                lastEvaluationResult = CM_ATTEN_CHAR;
                return lastEvaluationResult;
            }
            if (pHomePlugHdr->MMTYPE == (uint16_t)(CM_SLAC_MATCH | MMTYPE_CNF))
            {   // CM_SLAC_MATCH.CNF
                cm_slac_match_confirm* pMsg = (cm_slac_match_confirm*)receiveMessage;
                if (!checkMessage("CM_SLAC_MATCH.CNF", pHomePlugHdr->MMV, pMsg->APPLICATION_TYPE, pMsg->SECURITY_TYPE, values.RunID))
                {
                    lastEvaluationResult = CM_SLAC_INVALID;
                    return CM_SLAC_INVALID;
                }
                memcpy (values.EVSE_ID, pMsg->MatchVarField.EVSE_ID, SLAC_UNIQUE_ID_LEN);
                memcpy (values.NMK, pMsg->MatchVarField.NMK, SLAC_NMK_LEN);
                memcpy (values.NID, pMsg->MatchVarField.NID, SLAC_NID_LEN);
                dbg.printf(enDebugLevel_Debug, "Received CM_SLAC_MATCH.REQ . NID = %s, NMK = %s", ByteArrayToString((char*)values.NID, SLAC_NID_LEN).c_str(), ByteArrayToString((char*)values.NMK, SLAC_NID_LEN).c_str());
                lastEvaluationResult = CM_SLAC_MATCH;
                return lastEvaluationResult;
            }

            //////////////////

            //else
            {
                dbg.printf(enDebugLevel_Debug, "Received Unknown HomePlug AV message!", values.deviceVersion, cPacketSocket::mac_toString(qcaMAC).c_str());
                return 0xFFFE; // Message is HomePlugAV message but could not be evaluated.
            }
        }
    }
    return 0xFFFF; // Message not evaluated


//            {
//                if (memcmp(pMsg->ethernet.OSA, pSocket->getMacAddressLocal(), ETHER_ADDR_LEN)!=0)
//                {   // Only print non mirrored messages (happens at least for MAC_ATHEROSPING multicast MAC address)
//                    dbg.printf(enDebugLevel_Debug, "Unknown Homeplug AV message.");
//                }
//                else
//                {
//                    dbg.printf(enDebugLevel_Debug, "Mirror");
//                }
//                //pFramePort->receiveAcknowledge();
//                return; // return false;
//            }
//        else if (pMsg->ethernet.MTYPE == BigEndian((uint16_t)0x6003))
//        {
//            dbg.printf(enDebugLevel_Debug, "Received Codico: %s", ByteArrayToString((char*)pMsg, receiveMessageLen).c_str());
//            //pFramePort->receiveAcknowledge();
//            return; // return false;
//        }
//        else
//        {   // Discard non Homeplug AV messages
//            dbg.printf(enDebugLevel_Debug, "Non Homeplug AV message.");
//            //pFramePort->receiveAcknowledge();

    return false;
}

//void cPlcStackPub::receiveAcknowledge()
//{
//    pSocket->receiveAcknowledge();
//}
