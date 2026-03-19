/*====================================================================*
 *   Extracted from Qualcomm Open PLC Utils
 *
 *   slac.h, mme.h, types.h, plc.h, homeplug.h, qualcomm.h
 *
 *   this file contains defintions for the HomePlug Green PHY SLAC
 *   protocol
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted (subject to the limitations
 *   in the disclaimer below) provided that the following conditions
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *--------------------------------------------------------------------*/

#ifndef SLAC_MESSAGES_H
#define SLAC_MESSAGES_H

#define EVSE_NID "026BCBA5354E08"                       // HomePlugAV0123
#define EVSE_NMK "B59319D7E8157BA001B018669CCEE30D"     // HomePlugAV0123

#define EVSE_SID "7075626C696356324769736F3135313138" // Station Identifier (string of 17 hex values)
//#define EVSE_NMK "C5D78A54C98DE7AB903B658CEB824E4D"   // HomePlugAV0123
//#define EVSE_NID "9A3AC648EBC739"                     // HomePlugAV0123

#define PEV_VID "0000000000000000000000000000000000" // VehicleIdentifier
#define PEV_NMK "50D3E4933F855B7040784DF815AA8DB7"   // HomePlugAV
#define PEV_NID "B0F2E695666B03"                     // HomePlugAV

#ifndef __packed
#ifdef __GNUC__
#define __packed __attribute__ ((packed))
#else
#define __packed
#endif
#endif

#include <stdint.h>

#include "../../LibCpp/HAL/cPacketSocket.h"

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#ifndef UCHAR_MAX
#define UCHAR_MAX (0xFF)
#endif

//#define	ETHERMTU 1500

/*
 * The following two constants control whether or not the PEV or EVSE
 * change AVLN on SLAC protocol cycle; The recommended setting is PEV
 * changes with each pass and the EVSE does not;
 */

#define SLAC_AVLN_PEV 1
#define SLAC_AVLN_EVSE 0

#define SLAC_APPLICATION_PEV_EVSE 0x00

#define SLAC_SECURITY_NONE 0x00
#define SLAC_SECURITY_PUBLIC_KEY 0x01

#define SLAC_RUNID_LEN 8
#define SLAC_UNIQUE_ID_LEN 17
#define SLAC_RND_LEN 16
#define SLAC_NID_LEN 7
#define SLAC_NMK_LEN 16

#define SLAC_MSOUNDS 10
#define SLAC_TIMETOSOUND 6
#define SLAC_TIMEOUT 1000
#define SLAC_APPLICATION_TYPE 0
#define SLAC_SECURITY_TYPE 0
#define SLAC_RESPONSE_TYPE 1
#define SLAC_MSOUND_TARGET "FF:FF:FF:FF:FF:FF"
#define SLAC_FORWARD_STATION "00:00:00:00:00:00"
#define SLAC_GROUPS 58

#define SLAC_LIMIT 40
#define SLAC_PAUSE 20
#define SLAC_SETTLETIME 10
#define SLAC_CHARGETIME 2
#define SLAC_FLAGS 0

#define SLAC_SILENCE   (1 << 0)
#define SLAC_VERBOSE   (1 << 1)
#define SLAC_SESSION   (1 << 2)
#define SLAC_COMPARE   (1 << 3)
#define SLAC_SOUNDONLY (1 << 4)

#define SLAC_CM_SETKEY_KEYTYPE 0x01
#define SLAC_CM_SETKEY_PID 0x04
#define SLAC_CM_SETKEY_PRN 0x00
#define SLAC_CM_SETKEY_PMN 0x00
#define SLAC_CM_SETKEY_CCO 0x00
#define SLAC_CM_SETKEY_EKS 0x01

#define PLC_MODULE_SIZE 1400

#define ETH_MTYPE_HPAV 0x88E1       ///< Ethernet package message type (MTYPE) value for HomeplugAV

#define HOMEPLUG_MMV 0x01           ///< Homeplug AV constants
#define HOMEPLUG_MMTYPE 0x0000

#define CC_CCO_APPOINT 0x0000
#define CC_BACKUP_APPOINT 0x0004
#define CC_LINK_INFO 0x0008
#define CC_HANDOVER 0x000C
#define CC_HANDOVER_INFO 0x0010
#define CC_DISCOVER_LIST 0x0014
#define CC_LINK_NEW 0x0018
#define CC_LINK_MOD 0x001C
#define CC_LINK_SQZ 0x0020
#define CC_LINK_REL 0x0024
#define CC_DETECT_REPORT 0x0028
#define CC_WHO_RU 0x002C
#define CC_ASSOC 0x0030
#define CC_LEAVE 0x0034
#define CC_SET_TEI_MAP 0x0038
#define CC_RELAY 0x003C
#define CC_BEACON_RELIABILITY 0x0040
#define CC_ALLOC_MOVE 0x0044
#define CC_ACCESS_NEW 0x0048
#define CC_ACCESS_REL 0x004C
#define CC_DCPPC 0x0050
#define CC_HP1_DET 0x0054
#define CC_BLE_UPDATE 0x0058
#define CP_PROXY_APPOINT 0x2000
#define PH_PROXY_APPOINT 0x2004
#define CP_PROXY_WAKE 0x2008
#define NN_INL 0x4000
#define NN_NEW_NET 0x4004
#define NN_ADD_ALLOC 0x4008
#define NN_REL_ALLOC 0x400C
#define NN_REL_NET 0x4010
#define CM_ASSOCIATED_STA 0x6000
#define CM_ENCRYPTED_PAYLOAD 0x6004
#define CM_SET_KEY 0x6008
#define CM_GET_KEY 0x600C
#define CM_SC_JOIN 0x6010
#define CM_CHAN_EST 0x6014
#define CM_TM_UPDATE 0x6018
#define CM_AMP_MAP 0x601C
#define CM_BRG_INFO 0x6020
#define CM_CONN_NEW 0x6024
#define CM_CONN_REL 0x6028
#define CM_CONN_MOD 0x602C
#define CM_CONN_INFO 0x6030
#define CM_STA_CAP 0x6034
#define CM_NW_INFO 0x6038
#define CM_GET_BEACON 0x603C
#define CM_HFID 0x6040
#define CM_MME_ERROR 0x6044
#define CM_NW_STATS 0x6048
#define CM_SLAC_PARAM 0x6064
#define CM_START_ATTEN_CHAR 0x6068
#define CM_ATTEN_CHAR 0x606C
#define CM_PKCS_CERT 0x6070
#define CM_MNBC_SOUND 0x6074
#define CM_VALIDATE 0x6078
#define CM_SLAC_MATCH 0x607C
#define CM_SLAC_USER_DATA 0x6080
#define CM_ATTEN_PROFILE 0x6084

#define CM_SLAC_INVALID 0xFFFE
#define CM_SLAC_UNKNOWN 0xFFFF

#define MME_MMV 0                   ///< Atheros device command set (MME) version

#define MMTYPE_CC 0x0000
#define MMTYPE_CP 0x2000
#define MMTYPE_NN 0x4000
#define MMTYPE_CM 0x6000
#define MMTYPE_MS 0x8000
#define MMTYPE_VS 0xA000
#define MMTYPE_XX 0xC000

#define MMTYPE_REQ 0x0000
#define MMTYPE_CNF 0x0001
#define MMTYPE_IND 0x0002
#define MMTYPE_RSP 0x0003
#define MMTYPE_MODE  (MMTYPE_REQ|MMTYPE_CNF|MMTYPE_IND|MMTYPE_RSP)
#define MMTYPE_MASK ~(MMTYPE_REQ|MMTYPE_CNF|MMTYPE_IND|MMTYPE_RSP)

// Manufacturer specific implemented MMTYPES
#define VS_SW_VER 0xA000
#define VS_WR_MEM 0xA004
#define VS_RD_MEM 0xA008
#define VS_ST_MAC 0xA00C
#define VS_GET_NVM 0xA010
#define VS_RSVD_1 0xA014
#define VS_RSVD_2 0xA018
#define VS_RS_DEV 0xA01C
#define VS_WR_MOD 0xA020
#define VS_RD_MOD 0xA024
#define VS_MOD_NVM 0xA028
#define VS_WD_RPT 0xA02C
#define VS_LNK_STATS 0xA030
#define VS_SNIFFER 0xA034
#define VS_NW_INFO 0xA038
#define VS_RSVD_3 0xA03C
#define VS_CP_RPT 0xA040
#define VS_ARPC 0xA044
#define VS_SET_KEY 0xA050
#define VS_MFG_STRING 0xA054
#define VS_RD_CBLOCK 0xA058
#define VS_SET_SDRAM 0xA05C
#define VS_HOST_ACTION 0xA060
#define VS_OP_ATTRIBUTES 0xA068
#define VS_ENET_SETTINGS 0xA06C
#define VS_TONE_MAP_CHAR 0xA070
#define VS_NW_INFO_STATS 0xA074
#define VS_SLAVE_MEM 0xA078
#define VS_FAC_DEFAULTS 0xA07C
#define VS_MULTICAST_INFO 0xA084
#define VS_CLASSIFICATION 0xA088
#define VS_RX_TONE_MAP_CHAR 0xA090
#define VS_SET_LED_BEHAVIOR 0xA094
#define VS_WRITE_AND_EXECUTE_APPLET 0xA098
#define VS_MDIO_COMMAND 0xA09C
#define VS_SLAVE_REG 0xA0A0
#define VS_BANDWIDTH_LIMITING 0xA0A4
#define VS_SNID_OPERATION 0xA0A8
#define VS_NN_MITIGATE 0xA0AC
#define VS_MODULE_OPERATION 0xA0B0
#define VS_DIAG_NETWORK_PROBE 0xA0B4
#define VS_PL_LINK_STATUS 0xA0B8
#define VS_GPIO_STATE_CHANGE 0xA0BC
#define VS_CONN_ADD 0xA0C0
#define VS_CONN_MOD 0xA0C4
#define VS_CONN_REL 0xA0C8
#define VS_CONN_INFO 0xA0CC
#define VS_MULTIPORT_LNK_STA 0xA0D0
#define VS_EM_ID_TABLE 0xA0DC
#define VS_STANDBY 0xA0E0
#define VS_SLEEPSCHEDULE 0xA0E4
#define VS_SLEEPSCHEDULE_NOTIFICATION 0xA0E8
#define VS_MICROCONTROLLER_DIAG 0xA0F0
#define VS_GET_PROPERTY 0xA0F8
#define VS_SET_PROPERTY 0xA100
#define VS_PHYSWITCH_MDIO 0xA104
#define VS_SELFTEST_ONETIME_CONFIG 0xA10C
#define VS_SELFTEST_RESULTS 0xA110
#define VS_MDU_TRAFFIC_STATS 0xA114
#define VS_FORWARD_CONFIG 0xA118
#define VS_HYBRID_INFO 0xA200
#define VS_PEER_PING 0x0AC0

#define VAL_COOKIE 0x12345678

typedef unsigned char byte;

extern char MAC_ATHEROSPING[ETHER_ADDR_LEN];

extern const char* StationRole [3];

#ifndef __GNUC__
#pragma pack (push,1)
#endif

// ------------------------------------------------
// message elements
// ------------------------------------------------

//typedef struct __packed stMessage

//{
//    LibCpp::stEthernetHeader ethernet;
//    uint8_t content [ETHERMTU];
//} stMessage;

typedef struct __packed homeplug_hdr
{
    uint8_t MMV;
    uint16_t MMTYPE;
} homeplug_hdr;

typedef struct __packed homeplug_fmi
{
    uint8_t MMV;
    uint16_t MMTYPE;
    uint8_t FMSN;
    uint8_t FMID;
} homeplug_fmi;

typedef struct __packed qualcomm_hdr
{
    uint8_t MMV;
    uint16_t MMTYPE;
    uint8_t OUI [ETHER_ADDR_LEN >> 1];
} qualcomm_hdr;

typedef struct __packed qualcomm_fmi_v0
{
    uint8_t MMV;
    uint16_t MMTYPE;
    uint8_t OUI [ETHER_ADDR_LEN >> 1];
} qualcomm_fmi_v0;

typedef struct __packed qualcomm_fmi
{
    uint8_t MMV;
    uint16_t MMTYPE;
    uint8_t FMSN;
    uint8_t FMID;
    uint8_t OUI [ETHER_ADDR_LEN >> 1];
} qualcomm_fmi;

// --------------------------------------------------------------------
//   homeplug slac messages;
// --------------------------------------------------------------------

typedef struct __packed cm_sta_identity_request
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
} cm_sta_identity_request;

typedef struct __packed cm_sta_identity_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t GREEN_PHY_CAPABILITY;
    uint8_t POWER_SAVE_CAPABILITY;
    uint8_t GREEN_PHY_PREFERRED_ALLOCATION_CAPABILITY;
    uint8_t REPEATING_AND_ROUTING_CAPABILITY;
    uint8_t HOMEPLUG_AV_VERSION;
    uint8_t EFL;
    uint8_t EF [1];
} cm_sta_identity_confirm;

typedef struct __packed vs_link_status_request
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
} vs_link_status_request;

typedef struct __packed vs_link_status_response
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
    uint8_t MSTATUS;
    uint8_t LINKSTATUS;
} vs_link_status_confirm;

typedef struct __packed vs_peer_ping
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
} vs_peer_ping;

typedef struct __packed cm_slac_param_request
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    uint8_t RunID [SLAC_RUNID_LEN];
    uint8_t CipherSuiteSetSize;
    uint16_t CipherSuite [1];
} cm_slac_param_request;

typedef struct __packed cm_slac_param_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t MSOUND_TARGET [ETHER_ADDR_LEN];
    uint8_t NUM_SOUNDS;
    uint8_t TIME_OUT;
    uint8_t RESP_TYPE;
    uint8_t FORWARDING_STA [ETHER_ADDR_LEN];
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    uint8_t RunID [SLAC_RUNID_LEN];
    uint16_t CipherSuite;
} cm_slac_param_confirm;

typedef struct __packed cm_start_atten_char_indicate
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    struct __packed
    {
        uint8_t NUM_SOUNDS;
        uint8_t TIME_OUT;
        uint8_t RESP_TYPE;
        uint8_t FORWARDING_STA [ETHER_ADDR_LEN];
        uint8_t RunID [SLAC_RUNID_LEN];
    }
    ACVarField;
} cm_start_atten_char_indicate;

typedef struct __packed cm_start_atten_char_response
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
} cm_start_atten_char_response;

typedef struct __packed cm_atten_char_indicate
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    struct __packed
    {
        uint8_t SOURCE_ADDRESS [ETHER_ADDR_LEN];
        uint8_t RunID [SLAC_RUNID_LEN];
        uint8_t SOURCE_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t RESP_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t NUM_SOUNDS;
        struct __packed
        {
            uint8_t NumGroups;
            uint8_t AAG [255];
        }
        ATTEN_PROFILE;
    }
    ACVarField;
} cm_atten_char_indicate;

typedef struct __packed cm_atten_char_response
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    struct __packed
    {
        uint8_t SOURCE_ADDRESS [ETHER_ADDR_LEN];
        uint8_t RunID [SLAC_RUNID_LEN];
        uint8_t SOURCE_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t RESP_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t Result;
    }
    ACVarField;
} cm_atten_char_response;

typedef struct __packed cm_mnbc_sound_indicate
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    struct __packed
    {
        uint8_t SenderID [SLAC_UNIQUE_ID_LEN];
        uint8_t CNT;
        uint8_t RunID [SLAC_RUNID_LEN];
        uint8_t Reserved [8];               // Is missing in Qualcomm original files!
        uint8_t RND [SLAC_UNIQUE_ID_LEN];
    }
    MSVarField;
} cm_mnbc_sound_indcate;

typedef struct __packed cm_validate_request
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t SignalType;
    struct __packed
    {
        uint8_t Timer;
        uint8_t Result;
    }
    VRVarField;
} cm_validate_request;

typedef struct __packed cm_validate_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t SignalType;
    struct __packed
    {
        uint8_t ToggleNum;
        uint8_t Result;
    }
    VCVarField;
} cm_validate_confirm;

typedef struct __packed cm_slac_match_request
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    uint16_t MVFLength;
    struct __packed
    {
        uint8_t PEV_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t PEV_MAC [ETHER_ADDR_LEN];
        uint8_t EVSE_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t EVSE_MAC [ETHER_ADDR_LEN];
        uint8_t RunID [SLAC_RUNID_LEN];
        uint8_t RSVD [8];
    }
    MatchVarField;
} cm_slac_match_request;

typedef struct __packed cm_slac_match_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t APPLICATION_TYPE;
    uint8_t SECURITY_TYPE;
    uint16_t MVFLength;
    struct __packed
    {
        uint8_t PEV_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t PEV_MAC [ETHER_ADDR_LEN];
        uint8_t EVSE_ID [SLAC_UNIQUE_ID_LEN];
        uint8_t EVSE_MAC [ETHER_ADDR_LEN];
        uint8_t RunID [SLAC_RUNID_LEN];
        uint8_t RSVD1 [8];
        uint8_t NID [SLAC_NID_LEN];
        uint8_t RSVD2;
        uint8_t NMK [SLAC_NMK_LEN];
    }
    MatchVarField;
} cm_slac_match_confirm;

typedef struct __packed cm_atten_profile_indicate
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t PEV_MAC [ETHER_ADDR_LEN];
    uint8_t NumGroups;
    uint8_t RSVD;
    uint8_t AAG [255];
} cm_atten_profile_indicate;

typedef struct __packed cm_set_key_request
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t KEYTYPE;
    uint32_t MYNOUNCE;
    uint32_t YOURNOUNCE;
    uint8_t PID;
    uint16_t PRN;
    uint8_t PMN;
    uint8_t CCOCAP;
    uint8_t NID [SLAC_NID_LEN];
    uint8_t NEWEKS;
    uint8_t NEWKEY [SLAC_NMK_LEN];
    uint8_t RSVD [3];
} cm_set_key_request;

typedef struct __packed cm_set_key_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct homeplug_fmi homeplug;
    uint8_t RESULT;
    uint32_t MYNOUNCE;
    uint32_t YOURNOUNCE;
    uint8_t PID;
    uint16_t PRN;
    uint8_t PMN;
    uint8_t CCOCAP;
    uint8_t RSVD [27];
} cm_set_key_confirm;

//typedef struct __packed cc_discover_list_indicate
//{
//    LibCpp::stEthernetHeader ethernet;
//    struct homeplug_fmi homeplug;
//    uint8_t RESULT;
//    uint32_t MYNOUNCE;
//    uint32_t YOURNOUNCE;
//    uint8_t PID;
//    uint16_t PRN;
//    uint8_t PMN;
//    uint8_t CCOCAP;
//    uint8_t RSVD [27];
//} cc_discover_list_indicate;

// --------------------------------------------------------------------
//   qualcomm manufacturer specific messages
// --------------------------------------------------------------------

// plctool -ar
// "VersionInfo2"
typedef struct __packed vs_sw_ver_request
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
    uint32_t COOKIE;
} vs_sw_ver_request;

typedef struct __packed vs_sw_ver_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
    uint8_t MSTATUS;
    uint8_t MDEVICE_CLASS;
    uint8_t MVERLENGTH;
    char MVERSION [254];
    uint32_t IDENT;
    uint32_t STEPPING_NUM;
    uint32_t COOKIE;
    uint32_t RSVD [6];
} vs_sw_ver_confirm;

// plctool -aI
typedef struct __packed vs_module_operation_read_request
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
    uint32_t RESERVED;
    uint8_t NUM_OP_DATA;
    struct __packed
    {
        uint16_t MOD_OP;
        uint16_t MOD_OP_DATA_LEN;
        uint32_t MOD_OP_RSVD;
        uint16_t MODULE_ID;
        uint16_t MODULE_SUB_ID;
        uint16_t MODULE_LENGTH;
        uint32_t MODULE_OFFSET;
    }
    MODULE_SPEC;
} vs_module_operation_read_request;

typedef struct __packed vs_module_operation_read_confirm
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_hdr qualcomm;
    uint16_t MSTATUS;
    uint16_t ERR_REC_CODE;
    uint32_t RESERVED;
    uint8_t NUM_OP_DATA;
    struct __packed
    {
        uint16_t MOD_OP;
        uint16_t MOD_OP_DATA_LEN;
        uint32_t MOD_OP_RSVD;
        uint16_t MODULE_ID;
        uint16_t MODULE_SUB_ID;
        uint16_t MODULE_LENGTH;
        uint32_t MODULE_OFFSET;
    }
    MODULE_SPEC;
    uint8_t MODULE_DATA [PLC_MODULE_SIZE];
} vs_module_operation_read_confirm;

// Network participants (see plctool NetInfo2.c)
// plctool -m
typedef struct __packed vs_nw_info_request
{
    LibCpp::stEthernetHeader ethernet;
    struct qualcomm_fmi qualcomm;
} vs_nw_info_request;

typedef struct __packed vs_nw_info_confirm
{
    LibCpp::stEthernetHeader ethernet;
    union
    {
        struct __packed
        {
            struct qualcomm_fmi_v0 qualcomm;
            uint8_t content[1];
        } v0;
        struct __packed
        {
            struct qualcomm_fmi qualcomm;
            uint8_t  SUB_VERSION;
            uint8_t  Reserved;
            uint16_t DATA_LEN;
            uint8_t  DATA [1];
        } v1;
    };
} vs_nw_info_confirm;

typedef struct __packed station
{
    union
    {
        struct __packed
        {
            uint8_t MAC [ETHER_ADDR_LEN];
            uint8_t TEI;
            uint8_t BDA [ETHER_ADDR_LEN];
            uint8_t AVGTX;
            uint8_t AVGRX;
        } v0;
        struct __packed
        {
            uint8_t MAC [ETHER_ADDR_LEN];
            uint8_t TEI;
            uint8_t Reserved [3];
            uint8_t BDA [ETHER_ADDR_LEN];
            uint16_t AVGTX;
            uint8_t COUPLING;
            uint8_t Reserved3;
            uint16_t AVGRX;
            uint16_t Reserved4;
        } v1;
    };
} station;

typedef struct __packed network
{
    union
    {
        struct __packed
        {
            uint8_t NID [7];
            uint8_t SNID;
            uint8_t TEI;
            uint8_t ROLE;
            uint8_t CCO_MAC [ETHER_ADDR_LEN];
            uint8_t CCO_TEI;
            uint8_t NUMSTAS;
            struct station stations [1];
        } v0;
        struct __packed
        {
            uint8_t NID [7];
            uint8_t Reserved1 [2];
            uint8_t SNID;
            uint8_t TEI;
            uint8_t Reserved2 [4];
            uint8_t ROLE;
            uint8_t CCO_MAC [ETHER_ADDR_LEN];
            uint8_t CCO_TEI;
            uint8_t Reserved3 [3];
            uint8_t NUMSTAS;
            uint8_t Reserved4 [5];
            struct station stations [1];
        } v1;
    };
} network;

typedef struct __packed networks
{
    union
    {
        struct __packed
        {
            uint8_t NUMAVLNS;
            struct network networks [1];
        } v0;
        struct __packed
        {
            uint8_t Reserved;
            uint8_t NUMAVLNS;
            struct network networks [1];
        } v1;
    };
} networks;

#ifndef __GNUC__
#pragma pack (pop)
#endif

// ------------------------------------------------
// session data struct
// ------------------------------------------------

typedef struct session
{
    byte RunID [SLAC_RUNID_LEN];
    byte APPLICATION_TYPE;
    byte SECURITY_TYPE;
    byte RESP_TYPE;
    byte NUM_SOUNDS;
    byte TIME_OUT;
    byte AAG [SLAC_GROUPS];
    byte NumGroups;
    byte MSOUND_TARGET [ETHER_ADDR_LEN];
    byte FORWARDING_STA [ETHER_ADDR_LEN];
    byte PEV_ID [SLAC_UNIQUE_ID_LEN];
    byte PEV_MAC [ETHER_ADDR_LEN];
    byte EVSE_ID [SLAC_UNIQUE_ID_LEN];
    byte EVSE_MAC [ETHER_ADDR_LEN];
    byte RND [SLAC_UNIQUE_ID_LEN];
    byte NMK [SLAC_NMK_LEN];
    byte NID [SLAC_NID_LEN];
    byte original_nmk [SLAC_NMK_LEN];
    byte original_nid [SLAC_NID_LEN];
    unsigned state;
    unsigned sounds;
    unsigned limit;
    unsigned pause;
    unsigned chargetime;
    unsigned settletime;
    unsigned counter;
    unsigned flags;
    signed exit;
    char deviceVersion[254];
    char linkStatus;
    byte CCO_MAC [ETHER_ADDR_LEN];  ///< Central Coordinator MAC of QCA chip after call of sendVS_NW_INFO_REQ() of the network memorized in member NID
    byte STA_MAC [ETHER_ADDR_LEN];  ///< Station MAC of QCA chip after call of sendVS_NW_INFO_REQ() of the network memorized in member NID
} session;

#endif
