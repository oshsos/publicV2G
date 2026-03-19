/**
\file cWinPacketSocket.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-04-22

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Windows
@{

\class LibCpp::cWinPacketSocket
\brief Windows implementation of the \ref G_LibCpp_HAL hardware abstraction layer interface LibCpp::cPacketSocket class

// Link with Iphlpapi.lib
// Link with Ws2_32.lib
// Für Qt (*.pro):
//LIBS += $$PWD/LibCpp/HAL/Windows/iphlpapi.lib
//LIBS += $$PWD/LibCpp/HAL/Windows/ws2_32.lib

//C:\Users\Olaf Simon>ipconfig

// use 'SocketTest' for testing.


cWinPacketSocket port;
port.open();
port.send("sendBuffer"Hallo", 5);

int len = 0;
char receiveBuffer[1024];
while (!len)
{
    len = port.receive(receiveBuffer, 1024);
    printf("%s", receiveBuffer);
}

port.open(55000, "WLAN");

----------------------

#include <stdio.h>
#include <conio.h>

#include "../LibCpp/HAL/Windows/cWinPacketSocket.h"
#include "../LibCpp/HAL/Tools.h"

using namespace LibCpp;


cWinPacketSocket port;


int main()
{
    cDebug dbg("main");
    dbg.setDebugLevel();
    port.dbg.setInstanceName("port", &dbg);

    port.open(55000, "WLAN");

    char receiveBuffer[1024];
    while (1)
    {
        int len = port.receive(receiveBuffer, 1024);
        if (len)
            printf_flush("Received: %s\n", receiveBuffer);

        if (_kbhit())
        {
            input = getch();
            if (input == 'q')
                break;
            if (input == 's')
            {
                const char* sendBuffer = "Hallo";
                port.send(sendBuffer, strlen(sendBuffer));
                printf_flush("Sent    : %s\n", sendBuffer);
            }
        }
    }
    printf_flush("\nready\n");
    return 0;



//Windows-IP-Konfiguration


//Drahtlos-LAN-Adapter LAN-Verbindung* 1:

//   Medienstatus. . . . . . . . . . . : Medium getrennt
//   Verbindungsspezifisches DNS-Suffix:

//Drahtlos-LAN-Adapter LAN-Verbindung* 2:

//   Medienstatus. . . . . . . . . . . : Medium getrennt
//   Verbindungsspezifisches DNS-Suffix:

//Ethernet-Adapter VMware Network Adapter VMnet1:

//   Verbindungsspezifisches DNS-Suffix:
//   Verbindungslokale IPv6-Adresse  . : fe80::52cc:24f9:afc:a9dd%13
//   IPv4-Adresse  . . . . . . . . . . : 192.168.134.1
//   Subnetzmaske  . . . . . . . . . . : 255.255.255.0
//   Standardgateway . . . . . . . . . :

//Ethernet-Adapter VMware Network Adapter VMnet8:

//   Verbindungsspezifisches DNS-Suffix:
//   Verbindungslokale IPv6-Adresse  . : fe80::a30e:8447:d845:a8c6%19
//   IPv4-Adresse  . . . . . . . . . . : 192.168.19.1
//   Subnetzmaske  . . . . . . . . . . : 255.255.255.0
//   Standardgateway . . . . . . . . . :
**/

//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN

#define EFSU_ETHERTYPE 0x88E1
#define ETH_P_802_2 1
#define	ETHERMTU		1500
#define	ETHER_ADDR_LEN 6
#define OID_802_3_CURRENT_ADDRESS 0x01010102
#define	ETHER_MIN_LEN		64

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cWinPacketSocket.h"

#include "WpdPack/Include/pcap.h"
#include "WpdPack/Include/Packet32.h"

//#include "../Tools.h"
#include <chrono>


using namespace LibCpp;
using namespace std;

vector<string> getInterfaces()
{
    cDebug dbg("cWinPacketSocket::getInterfaces");

    vector<string> list;

    pcap_if_t *devices = nullptr;
    char buffer [PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs (&devices, buffer) == -1)
    {
        dbg.printf(LibCpp::enDebugLevel_Fatal, "Can't enumerate pcap devices! %s.", buffer);
    }

    for (pcap_if_t *device = devices; device != nullptr; device = device->next)
        list.push_back(device->name);

    pcap_freealldevs (devices);
    return list;
}

void LibCpp::printPcapInterfaces()
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i=0;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* Retrieve the device list from the local machine */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
        return;
    }

    /* Print the list */
    for(d= alldevs; d != NULL; d= d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }

    if (i == 0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return;
    }

    /* We don't need any more the device list. Free it */
    pcap_freealldevs(alldevs);
}

char* getifname (signed index)
{
    char * name = (char *)(0);

    char buffer [PCAP_ERRBUF_SIZE];
    pcap_if_t *devices = (pcap_if_t *)(0);
    pcap_if_t *device;
    signed count;
    if (pcap_findalldevs (&devices, buffer) == -1)
    {
        fprintf (stderr, "can't enumerate pcap devices");
    }
    for (device = devices, count = 1; device; device = device->next, count++)
    {
        if (count == index)
        {
            name = _strdup (device->name);
            break;
        }
    }
    if (!device)
    {
        fprintf(stderr,"invalid interface: %d", index);
    }
    pcap_freealldevs (devices);
    return (name);
}

int gethwaddr (void * memory, char const * device)
{
    cDebug dbg("cWinPacketSocket::gethwaddr");

    LPADAPTER adapter = PacketOpenAdapter ((PCHAR)(device));
    PPACKET_OID_DATA data = (PPACKET_OID_DATA)(malloc (ETHER_ADDR_LEN + sizeof (PACKET_OID_DATA)));
    if (!data)
    {
        dbg.printf(LibCpp::enDebugLevel_Fatal, "Can't allocate memory: %s", device);
        memset(memory, 0, ETHER_ADDR_LEN);
        return -1;
    }
    data->Oid = OID_802_3_CURRENT_ADDRESS;
    data->Length = ETHER_ADDR_LEN;
    if ((adapter == 0) || (adapter->hFile == INVALID_HANDLE_VALUE))
    {
        dbg.printf(LibCpp::enDebugLevel_Fatal, "Can't access interface: %s! Make sure WinPcap is installed!", device);
        PacketCloseAdapter (adapter);
        free (data);
        memset(memory, 0, ETHER_ADDR_LEN);
        return (-1);
    }
    if (!PacketRequest (adapter, FALSE, data))
    {
        memset(memory, 0, ETHER_ADDR_LEN);
        PacketCloseAdapter (adapter);
        free (data);
        return (-1);
    }
    memcpy_s(memory, ETHER_ADDR_LEN, data->Data, data->Length);
    PacketCloseAdapter (adapter);
    free (data);
    return 0;
}


/**
 * @brief Constructor
 */
cWinPacketSocket::cWinPacketSocket(bool open, string localInterfaceName, bool async, iFramePort* pCallback) :
    dbg("cWinPacketSocket")
{
    pThread = nullptr;
    pLocalSocket = nullptr;
    this->localInterfaceName = localInterfaceName;
    cWinPacketSocket::close();
    if (pCallback)
        setCallback(pCallback);
    if (open)
        cWinPacketSocket::open(localInterfaceName, async);
}

/**
 * @brief Destructor
 */
cWinPacketSocket::~cWinPacketSocket()
{
    cWinPacketSocket::close();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * This way several objects requiring to finish a background task can do so in parallel.
 * Just calling 'close' requires much time as each 'close' needs to wait for the backgrond task actually
 * having finished.
 */
void cWinPacketSocket::prepareClose()
{
    threadEnabled = false;
}

/**
 * @brief Closes or releases hardware ressources.
 * See LibCpp::cPacketSocket::close.
 */
void cWinPacketSocket::close()
{
    cDebug dbg("close", &this->dbg);

    threadEnabled = false;
    if (pThread)
    {
        pThread->join();
        delete pThread;
        pThread = nullptr;
    }
    if (pLocalSocket != nullptr)
    {
        pcap_close(pLocalSocket);
        pLocalSocket = nullptr;
        dbg.printf(enDebugLevel_Info, "Closed Packet Socket.");
    }
    memset(&localMacAddress, 0, ETHER_ADDR_LEN);
    messageLen = 0;
    bufferLen = 0;
    async = false;
    threadRunning = false;
}

/**
 * @brief Opens or aquires hardware ressources.
 * See LibCpp::cPacketSocket::open.
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param family
 * @param linkLocal If set and IPv6, only link local addresses (fe80:...) are accepted. Otherwise any address is taken.
 * @return Error code
 */
enIpResult cWinPacketSocket::open(string localInterfaceName, bool async)
{
    cDebug dbg("open", &this->dbg);

    if (localInterfaceName.empty())
        localInterfaceName = this->localInterfaceName;
    this->localInterfaceName = localInterfaceName;

    cInterfaces interfaces(true);
    stInterfaceInfo ifc = interfaces.getInterfaceInfo(localInterfaceName);
    ifName = string("\\Device\\NPF_") + ifc.name;
    interfaces.close();

//    int pcapIndex = atoi(localInterfaceName.c_str());
//    char* ifName = getifname (5);

    gethwaddr (&localMacAddress, ifName.c_str());
    this->async = async;

    threadEnabled = true;     // To be fast enough in case a direct 'receive' follows after 'open'
    threadRunning = false;
    pThread = new std::thread(&cWinPacketSocket::threadReceiving, this);

    while(threadEnabled && !threadRunning)
        this_thread::sleep_for(chrono::milliseconds(1));

    if (threadRunning)
        return enIpResult_Success;
    else
        return enIpResult_InvalidInterface;

}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sends a message to a unSockAddr type destination.
 * @param message
 * @param messageLen
 * @param blockingMode
 * @return Len of actually sent bytes, -1 on failure.
 */
int cWinPacketSocket::send(const char* message, int messageLen, enBlocking blockingMode)
{
    if (pLocalSocket)
    {
        int len = messageLen;
        if (pcap_sendpacket(pLocalSocket, (const u_char*)message, messageLen) != 0)
        {
            cDebug dbg("sendToInternal", &this->dbg);
            dbg.printf(enDebugLevel_Fatal,"Error sending the packet: %s! Check ethernet connection to be active!", pcap_geterr(pLocalSocket));
            return -1;
        }
        return len;
    }
    return -1;
}

/**
 * @brief Receives a message
 * @param buffer
 * @param bufferSize
 * @param blockingMode Not implemented, method is not blocking
 * @return Number of bytes received.
 */
int cWinPacketSocket::receive(char* buffer, int bufferSize, enBlocking blockingMode)
{
    int len = messageLen;
    if (len>0)
    {
        if (len>bufferSize) len=bufferSize;
        memcpy(buffer, message, len);
        if (len < bufferSize-1) buffer[len]=0;
        if (!callbacks.size())
            messageLen = 0;
    }
    return len;
}

/**
 * @brief Receives a message without copying from the receive buffer
 * In case the message is processed, call LibCpp::cLxUdpSocket::receiveAcknowledge to free the receive buffer.
 * @param ppBuffer
 * @param blockingMode blockingMode Not implemented, method is not blocking
 * @return Number of bytes available in the receive buffer.
 */
int cWinPacketSocket::receiveBuffer(char** ppBuffer, enBlocking blockingMode)
{
    *ppBuffer = message;
    return messageLen;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Frees the receive Buffer
 * Method is to be called after calling LibCpp::cLxUdpSocket::receiveBuffer to enable the reception of further messages.
 */
void cWinPacketSocket::receiveAcknowledge()
{
    if (messageLen>0 && !callbacks.size())              // do not access for writing, if the receiving task has control of the buffer
        messageLen = 0;
}

/**
 * @brief Method to be called cycelic to synchronously call registered callbacks
 * This type of operation mode is a polling method.
 */
void cWinPacketSocket::operate()
{
    if (!pLocalSocket || async)
        return;

//    struct pcap_pkthdr* header;
//    const u_char* pkt_data;
//    int res = pcap_next_ex( pLocalSocket, &header, &pkt_data);
//    if (res>0)
//    {
//        int len = header->len;
//        if (len > LibCpp_cIp_PACKET_BUFFERSIZE) len = LibCpp_cIp_PACKET_BUFFERSIZE;
//        if (len < 0) len = 0;
//        memcpy(message, pkt_data, len);
//        // Ensure zero termination of message
//        if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
//            message[len] = 0;
//        messageLen = len;
//    }

    internalOperate();
}

/**
 * @brief Method to call registered callback objects
 * The method is called either by 'operate' or the background thread
 * depending of 'async' is unset or set.
 */
void cWinPacketSocket::internalOperate()
{
    if (messageLen && callbacks.size())
    {
        for (iFramePort* call : callbacks)
            call->onFrameReceive((cFramePort*)this);
        messageLen = 0;
    }
}

/**
 * @brief Background thread for socket message reception
 */
void __cdecl packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

void cWinPacketSocket::threadReceiving()
{
    threadEnabled = true;
    threadRunning = true;

    cDebug dbg("threadReceiving", &this->dbg);

    char errbuf[PCAP_ERRBUF_SIZE] = "";
    /* Open the output device */
    if ( (pLocalSocket = pcap_open_live(
             ifName.c_str(),                 // name of the device (number string of interface list)
             LibCpp_cIp_PACKET_BUFFERSIZE,   // portion of the packet to capture (only the first 100 bytes)
             0,                              // promiscuous mode
             100,                            // read timeout (100), -1 = no timeout
             errbuf                          // error buffer
             ) ) == NULL)
    {
        dbg.printf(enDebugLevel_Fatal, "Opening packet socket at interface '%s' failed!", localInterfaceName.c_str());
        return;
    }

    dbg.printf(enDebugLevel_Info, "Listening as packet socket at interface '%s'.", localInterfaceName.c_str());

    while(threadEnabled)
    {
        int res = 0;

        res = pcap_dispatch(pLocalSocket, 1, packet_handler, (u_char*)this);
        if (res<0)
            dbg.printf(enDebugLevel_Debug, "Packet reception loop has been stopped!");

        if (bufferLen)
        {
            if (messageLen>0)
                dbg.printf(enDebugLevel_Debug, "Message buffer overrun.");
            while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
                this_thread::sleep_for(chrono::milliseconds(1));
            if (messageLen == 0)
            {
                //dbg.printf(enDebugLevel_Debug, "Putted packet to message:\n%s", ByteArrayToString(buffer, 10).c_str());
                memcpy(message, buffer, bufferLen);
                if (bufferLen < LibCpp_cIp_PACKET_BUFFERSIZE)
                    message[bufferLen] = 0;
                messageLen = bufferLen;
                if (async)
                {
                    internalOperate();
                }
            }
            bufferLen = 0;
        }
    }
    threadRunning = false;
    return;
}

void __cdecl packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
//    struct tm ltime;
//    char timestr[16];
//    time_t local_tv_sec;

    /*
     * unused variables
     */
//    (VOID)(param);
//    (VOID)(pkt_data);

    /* convert the timestamp to readable format */
//    local_tv_sec = header->ts.tv_sec;
//    localtime_s(&ltime, &local_tv_sec);
//    strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);

//    printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);

    cWinPacketSocket* pPacketSocket = (cWinPacketSocket*)param;

    unsigned int len = header->len;
    if (len > LibCpp_cIp_PACKET_BUFFERSIZE) len = LibCpp_cIp_PACKET_BUFFERSIZE;
    memcpy(pPacketSocket->buffer, pkt_data, len);
    // Ensure zero termination of message
    if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
        pPacketSocket->buffer[len] = 0;
    // Check for messaged not being mirrored
    stEthernetHeader* pHdr = (stEthernetHeader*)pPacketSocket->buffer;
    if (memcmp(pHdr->OSA, pPacketSocket->getMacAddressLocal(), ETHER_ADDR_LEN) == 0)
        len = 0;
    if (len)
    {
        // Check message to be a relevant one
        bool relevant = true;
        if (pPacketSocket->callbacks.size() && len)
        {
            relevant = false;
            for (iFramePort* pPort : pPacketSocket->callbacks)
                if (pPort->onReceiveFrameCheck(pPacketSocket, pPacketSocket->buffer, &len))
                    relevant = true;
        }
        if (relevant)
        {
            //pPacketSocket->dbg.printf(enDebugLevel_Debug, "********** Received relevant packet:\n%s", ByteArrayToString(pPacketSocket->buffer, 10).c_str());
            pPacketSocket->bufferLen = len;
            return;
        }
    }
    pPacketSocket->bufferLen = 0;
}



//        struct pcap_pkthdr* header;
//        const u_char* pkt_data;
//        int res = 1;


//        // Receive a message
//        res = pcap_next_ex( pLocalSocket, &header, &pkt_data);
//        if (res>0)
//        {
//            len = header->len;
//            if (len > LibCpp_cIp_PACKET_BUFFERSIZE) len = LibCpp_cIp_PACKET_BUFFERSIZE;
//            memcpy(buffer, pkt_data, len);
//            // Ensure zero termination of message
//            if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
//                buffer[len] = 0;
//            // Check for messaged not being mirrored
//            stEthernetHeader* pHdr = (stEthernetHeader*)buffer;
//            if (memcmp(pHdr->OSA, getMacAddressLocal(), ETHER_ADDR_LEN) == 0)
//                len = 0;
//            if (len)
//            {
//                // Check message to be a relevant one
//                bool relevant = true;
//                if (callbacks.size() && len)
//                {
//                    relevant = false;
//                    for (iFramePort* pPort : callbacks)
//                        if (pPort->onReceiveFrameCheck(this, buffer, &len))
//                            relevant = true;
//                }
//                if (relevant)
//                {
//                    if (messageLen>0)
//                        dbg.printf(enDebugLevel_Debug, "Message buffer full.");
//                    while (messageLen > 0 && threadEnabled)  // wait for message buffer to get read
//                        this_thread::sleep_for(chrono::milliseconds(1));
//                    if (messageLen == 0)
//                    {
//                        memcpy(message, buffer, len);
//                        if (len < LibCpp_cIp_PACKET_BUFFERSIZE)
//                            message[len] = 0;
//                        messageLen = len;
//                        if (async)
//                        {
//                            internalOperate();
//                        }
//                    }
//                }
//            }
//        }
//        else if (res < 0)
//        {
//            dbg.printf(enDebugLevel_Error, "Error reading the packets: %s\n", pcap_geterr(pLocalSocket));
//            threadEnabled = false;
//        }

/**
 * @brief Retrieves the local host IP address the object is receiving on.
 * @return
 */
unsigned char* cWinPacketSocket::getMacAddressLocal()
{
    return localMacAddress;
}

/**
 * @brief Sets the socket messaage filter (e.g. receives just messages to own ethernet (mac) address)
 */
void cWinPacketSocket::setFilter()
{
    cDebug dbg("setFilter", &this->dbg);

    struct bpf_program bpf_program;
    static struct bpf_insn bpf_insn [] =
        {
            {
                BPF_LD + BPF_H + BPF_ABS,
                0,
                0,
                12
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                18,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                0
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                10,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                1
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                8,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                2
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                6,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                3
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                4,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                4
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                2,
                0
            },
            {
                BPF_LD + BPF_B + BPF_ABS,
                0,
                0,
                5
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                4,
                0,
                0
            },
            {
                BPF_LD + BPF_W + BPF_ABS,
                0,
                0,
                0
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                4,
                0xFFFFFFFF
            },
            {
                BPF_LD + BPF_H + BPF_ABS,
                0,
                0,
                4
            },
            {
                BPF_JMP + BPF_JEQ + BPF_K,
                0,
                2,
                0xFFFF
            },
            {
                BPF_LD + BPF_W + BPF_LEN,
                0,
                0,
                0
            },
            {
                BPF_RET + BPF_A,
                0,
                0,
                0
            },
            {
                BPF_RET + BPF_K,
                0,
                0,
                0
            }
        };

    bpf_program.bf_len = sizeof (bpf_insn)/sizeof (struct bpf_insn);
    bpf_program.bf_insns = bpf_insn;
    if (EFSU_ETHERTYPE == ETH_P_802_2)
    {
        bpf_insn [1].code = BPF_JMP + BPF_JGT + BPF_K;
        bpf_insn [1].jt = 18;
        bpf_insn [1].jf = 0;
        bpf_insn [1].k = ETHERMTU;
    }
    else
    {
        bpf_insn [1].code = BPF_JMP + BPF_JEQ + BPF_K;
        bpf_insn [1].jt = 0;
        bpf_insn [1].jf = 18;
        bpf_insn [1].k = EFSU_ETHERTYPE;
    }
    bpf_insn [3].k = localMacAddress[0];
    bpf_insn [5].k = localMacAddress[1];
    bpf_insn [7].k = localMacAddress[2];
    bpf_insn [9].k = localMacAddress[3];
    bpf_insn [11].k = localMacAddress[4];
    bpf_insn [13].k = localMacAddress[5];
    if (pcap_setfilter(pLocalSocket, &bpf_program) < 0)
    {
        dbg.printf (enDebugLevel_Error, "Can't store pcap filter!");
    }
    if (pcap_setmintocopy(pLocalSocket, ETHER_MIN_LEN) < 0)
    {
        dbg.printf (enDebugLevel_Error, "Can't set pcap mintocopy!");
    }
}

/** @} */
