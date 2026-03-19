// UTF8 (ü) //
/**
\file   cTcpServer.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-10
\brief  See cTcpServer.h

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

@code
#define INTERFACE_NAME "Ethernet"
#define BUFFER_SIZE 1024

#include <stdio.h>
#include <conio.h>

#include "../LibCpp/HAL/Tools.h"

using namespace LibCpp;

char input;

#include "../LibCpp/HAL/Windows/cWinTcpServer.h"

cWinTcpServer server;

int main()
{
    cDebug dbg("main");
    dbg.setDebugLevel();
    server.dbg.setInstanceName("server", &dbg);

    printf_flush("press 'q' for quit and 's' for send and 'd' for disconnect \n\n");

    // Starting server operation
    server.open(55100, INTERFACE_NAME, LibCpp::enAddressFamily_IPv6);

    cConnectedClient* pClient = nullptr;
    char receiveBuffer[BUFFER_SIZE];
    while (1)
    {
        // Tcp server accept operation (wait for a client to connect)
        cConnectedClient* pNewClient = server.newClient();
        if (pNewClient)
        {
            if (!pClient)
            {
                pClient = pNewClient;
                dbg.printf(enDebugLevel_Info, "Client connected to server from %s.", pClient->getIpAddressRemote().toString(true, true).c_str());
            }
            else
            {
                dbg.printf(enDebugLevel_Error, "Connection refused!");
                delete pNewClient;
            }
        }

        // Receiving and disconnecting from the connected client
        if (pClient)
        {
            int len = pClient->receive(receiveBuffer, BUFFER_SIZE);
            if (len>0)
                printf_flush("Received: %s\n", receiveBuffer);
            else if (len==-1)
            {
                printf_flush("Client disconnected from server.\n");
                delete pClient;
                pClient = nullptr;
            }
        }

        if (_kbhit())
        {
            input = getch();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 's')
            {
                const char* sendBuffer = "Hallo";
                int len = cConnectedClient::send(&pClient, sendBuffer, strlen(sendBuffer));
                if (len>0)
                    printf_flush("Sent    : %s\n", sendBuffer);
                else
                    printf_flush("Sending impossible as client is not connected!\n");
            }
            if (input == 'd')
            {
                if (pClient)
                {
                    printf_flush("Server disconnects the client.\n");
                    delete pClient;
                    pClient = nullptr;
                }
            }
        }
    }

    if (pClient)
        delete pClient;
    server.close();
    printf_flush("\nready\n");
    return 0;
}
@endcode

\class LibCpp::cTcpServer
**/

#ifndef CTCPSERVER_H
#define CTCPSERVER_H

#include "cIp.h"
#include "cFramePort.h"

namespace LibCpp
{

/**
*   @brief Abstract interface class representing clients connected to the TCP server
*/
class cConnectedClient : public cFramePort
{
public:
    cConnectedClient();                         ///< Constructor
    virtual ~cConnectedClient();                ///< Destructor

    virtual int         send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///> Sends a data frame.
    virtual int         receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///> Receives a data frame.

    virtual bool        open();                 ///< Starts the client to be in active operation.
    virtual bool        isOpen();               ///< Checks for the client to be in active operation.
    virtual bool        prepareClose();         ///< Prepares for closing by friendly closing the background task.
    virtual stIpAddress getIpAddressRemote();   ///< Retrieves the IP address of the remote client represented by this instance.
    virtual int         getSocketDescriptor();  ///> Returns the socket file descriptor

    static int          send(cConnectedClient** ppClient, const char* message, int messageLength);      ///< Sends messages in case the client exists.
    static int          receive(cConnectedClient** ppClient, char* message, int messageSize);           ///< Receives messages and deletes the client on disconnection.
    static int          send(cConnectedClient** ppClient, const uint8_t* message, int messageLength);   ///< Sends messages in case the client exists.
    static int          receive(cConnectedClient** ppClient, uint8_t* message, int messageSize);        ///< Receives messages and deletes the client on disconnection.

    void                setExternalSndRcv(iExternalSndRcv* pExternalSndRcv);                            ///< Sets a callback instance to provide alternative 'send' and 'receive' methods

public:
    cDebug              dbg;                                ///< Logger intstance
    bool                showMessages;                       ///< If true, all messages are shown in debug mode.

protected:
    iExternalSndRcv*    pExternalSndRcv;                    ///< External class used to execute send and receive operations (required for TLS).
};

/**
*   @brief Abstract interface class of a TCP server
*/
class cTcpServer
{
public:
    cTcpServer();                                       ///< Constructor
    ~cTcpServer();                                      ///< Destructor

    virtual enIpResult          open(int localPort = LibCpp_cIp_TCP_PORT, std::string localInterfaceOrAddressName = "", enAddressFamily family = enAddressFamily_IPv4, bool linkLocal = true, int acceptedClients = 0, bool openClients = true);  ///< Sets the Tcp server into operational mode
    virtual void                prepareClose();         ///< Prepares for closing by friendly closing the background task.
    virtual void                close();                ///< Releases recources and stops background tasks.
    virtual cConnectedClient*   newClient();            ///< (Equals the call to 'accept'.)
    virtual cConnectedClient&   client();               ///< Access to the internal connected client (useful, if 'acceptedClients' equals 1).
    virtual stIpAddress         getIpAddressLocal();    ///< Deliveres the interface ip address the UDP socket is bound to.
    virtual int                 getLocalPort();         ///< Deliveres the port the object is listening on.

    bool                        newSingleClient(cConnectedClient** ppClient); ///< Accepts a new client if pClient=nullptr, otherwise deletes new clients.

public:
    bool                showMessages;                       ///< If true, all messages are shown in debug mode.

private:
    cConnectedClient            internalClient;         ///< Client instance being used if only one client is allowed to connect.
};

}

#endif

/** @} */
