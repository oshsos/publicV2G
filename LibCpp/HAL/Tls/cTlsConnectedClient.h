// UTF8 (ü) //
/**
\file   cTlsConnectedClient.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-10
\brief  See cWinTcpServer.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Tls
@{

\class LibCpp::cTlsConnectedClient

Take care to set a compiler include path to the path where the folder 'openssl' of the openSSL library header files is located.
- INCLUDEPATH += ../LibCpp/HAL/Tls
Furthermore the linker needs to use the openSSL dynamic libraries.
- LIBS += ../LibCpp/HAL/Tls/libcrypto-3-x64.dll
- LIBS += ../LibCpp/HAL/Tls/libssl-3-x64.dll

\code
#include "../LibCpp/HAL/Windows/cWinTcpServer.h"
#include "../LibCpp/HAL/Tls/cTlsConnectedClient.h"

#define BUFFER_SIZE 1024

cWinTcpServer server;
cTls::stCertFiles tlsConfig("certificates/server-cert.pem", "certificates/server-private.key", "certificates/ca-cert.pem");

int main()
{
    cDebug dbg("main");
    dbg.setDebugLevel();
    server.dbg.setInstanceName("server", &dbg);

    printf_flush("press 'q' for quit and 's' for send and 'd' for disconnect \n\n");

    // Starting server operation
    server.open(55100, INTERFACE_NAME, enAddressFamily_IPv4);
    //server.open(55100, "127.0.0.1");
    server.openClients = false;

    cTlsConnectedClient* pTlsClient = nullptr;
    char receiveBuffer[BUFFER_SIZE];
    while (1)
    {
        // Tcp server accept operation (wait for a client to connect)
        cConnectedClient* pNewTcpClient = server.newClient();
        if (pNewTcpClient)
        {   // A new client connection occured.
            if (!pTlsClient)
            {   // A new client can be accepted by the application.
                pTlsClient = cTlsConnectedClient::newClient(pNewTcpClient, tlsConfig);     // pClient takes control over pNewTcpClient and its deletion.
                if (pTlsClient)
                    dbg.printf(enDebugLevel_Info, "TLS client connected to this server from %s.", pTlsClient->getIpAddressRemote().toString(true, true).c_str());
                else
                {
                    dbg.printf(enDebugLevel_Info, "TLS client refused this server.");
                }
            }
            else
            {
                // This sample program just handles a single client connection
                dbg.printf(enDebugLevel_Info, "Server has a connected client already and refused connection from %s!", pNewTcpClient->getIpAddressRemote().toString(true, true).c_str());
                delete pNewTcpClient;
            }

            //            // Alternative approach without hidden instance allocation
            //            if (!pTlsClient)
            //            {
            //                pTlsClient = new cTlsConnectedClient(pNewTcpClient, tlsConfig);  // pClient takes control over pNewClient and its deletion.
            //                if (pTlsClient->tlsAccept())
            //                    dbg.printf(enDebugLevel_Info, "TLS client connected to this server from %s.", pTlsClient->getIpAddressRemote().toString(true, true).c_str());
            //                else
            //                {
            //                    dbg.printf(enDebugLevel_Info, "TLS client connected from %s has not accepted this server.", pTlsClient->getIpAddressRemote().toString(true, true).c_str());
            //                    delete pTlsClient;         // also deletes pNewTcpClient
            //                    pNewTcpClient = nullptr;
            //                }
            //            }
            //            else
            //            {
            //                dbg.printf(enDebugLevel_Info, "Server has a connected client already and refused connection from %s!", pNewTcpClient->getIpAddressRemote().toString(true, true).c_str());
            //                delete pNewTcpClient;
            //            }
        }

        // Receiving from and disconnection of the connected client
        if (pTlsClient)
        {
            int len = pTlsClient->receive(receiveBuffer, BUFFER_SIZE);
            if (len>0)
                printf_flush("Received: %s\n", receiveBuffer);
            else if (len==-1)
            {
                printf_flush("Client disconnected from server.\n");
                delete pTlsClient;
                pTlsClient = nullptr;
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
                if (pTlsClient)
                {
                    const char* sendBuffer = "Hallo";
                    int len = pTlsClient->send(sendBuffer, strlen(sendBuffer));
                    if (len>0)
                        printf_flush("Sent    : %s\n", sendBuffer);
                }
                else
                    printf_flush("Sending impossible as client is not connected!\n");
            }
            if (input == 'd')
            {
                if (pTlsClient)
                {
                    printf_flush("Server disconnects the client.\n");
                    delete pTlsClient;
                    pTlsClient = nullptr;
                }
            }
        }
    }

    if (pTlsClient)
        delete pTlsClient;
    server.close();
    printf_flush("\nready\n");
    return 0;
}
\endcode
**/

#ifndef CTLSCONNECTEDCLIENT_H
#define CTLSCONNECTEDCLIENT_H

#include "cTls.h"
#include "../cFramePort.h"
#include "../cTcpServer.h"
#include "../cDebug.h"

namespace LibCpp
{

/**
 * @brief This class represents a remote client connected to this server communicating with TLS encryption.
 */
class cTlsConnectedClient  : public cConnectedClient, public iExternalSndRcv
{
public:
    cTlsConnectedClient(cConnectedClient* pClient, cTls::stCertFiles tlsConfig);                                                        ///< Constructor
    virtual ~cTlsConnectedClient();                                                                                                     ///< Destructor

    static cTlsConnectedClient* newClient(cConnectedClient* pNewTcpClient, cTls::stCertFiles tlsConfig);                          ///< Returns a newly created cTlsConnectedClient instance if the client accepted this server, otherwise a null pointer other is returned and pClient is destroyed.

    bool                tlsAccept();

    virtual int         send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///< Sends a data frame.
    virtual int         receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///< Receives a data frame.
    virtual int         receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///< Receives a data frame without copying
    virtual void        receiveAcknowledge();                                       ///< Acknowledges a reception of a message by 'receiveBuffer'
    virtual void        operate();                                                  ///< Cyclic operation on callback polling mode usage.
    virtual bool        setCallback(iFramePort* pInstance);                         ///< Sets an instance to be called at message receptions.
    virtual bool        deleteCallback (iFramePort* pInstance = nullptr);           ///< Removes an instance from the callback list.
    virtual bool        setFrameCheckCallback (iFrameCheck* pInstance = nullptr);   ///< Sets an instance to control data framing and encoding.

    virtual bool        open();                 ///< Check for the client to be in active operation
    virtual bool        prepareClose();         ///< Prepares for closing by friendly closing the background task.
    virtual stIpAddress getIpAddressRemote();   ///< Retrieves the IP address of the remote client represented by this instance.

    virtual int         getSocketDescriptor();  ///> Returns the socket file descriptor

    inline bool         isInitialized() {return pSSL != nullptr;};
    inline bool         isAccepted() {return isAcceptedFlag;};

    int                 onExternalReceive(char* message, int bufferSize);
    int                 onExternalSend(const char* message, int messageSize);

public:
    cDebug              dbg;
protected:
    cConnectedClient*   pClient;

    bool                isAcceptedFlag;

    SSL_CTX*            pCTXSSL;       ///< SSL Context Data Structure
    SSL*                pSSL;          ///< SSL Data Structure
    X509*               pPeerCert;     ///< Presented certificated by the client, if any.
    cTls::stCertFiles   tlsConfig;     ///< Tls configuration data to be used
};

}

#endif
/** @} */
