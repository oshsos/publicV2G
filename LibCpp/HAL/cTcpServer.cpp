// UTF8 (ü) //
/**
\file cTcpServer.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-10

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cTcpServer
\brief Virtual class of an TCP protocol server

 */

#ifdef PROJECTDEFS
    #include <ProjectDefs.h>
#endif

#ifndef LibCpp_cTcpServer_MAXCLIENTS
    #define LibCpp_cTcpServer_MAXCLIENTS 10
#endif

#include "cTcpServer.h"

using namespace LibCpp;
using namespace std;

#pragma GCC diagnostic ignored "-Wunused-parameter"

/** @brief Constructor */
cConnectedClient::cConnectedClient()
{
    pExternalSndRcv = nullptr;
    showMessages = false;
}

/** @brief Destructor */
cConnectedClient::~cConnectedClient() {}

/**
 * @brief Checks the instance to be in active state.
 * This method is useful for the LibCpp::cTcpServer class internal client instance,
 * as this instance remains eben the remote client has disconnected. This method can
 * be used for polling purpose, if the remote client has reconnected.
 */
bool cConnectedClient::open() {return false;}

/**
 * @brief Checks for the client to be in active operation.
 * @return
 */
bool cConnectedClient::isOpen() {return false;}

/**
 * @brief Prepares closing respectively destruction by initiating the friendly ending of the background task.
 * @return 'true' in case the closing process is finished (polling option before calling 'close' call.)
 */
bool cConnectedClient::prepareClose() {return true;}

/** @brief Sends a message
 *  See LibCpp::cFramePort::send
 */
int cConnectedClient::send(const char* pMsg, int messageLen, enBlocking blockingMode) {return 1;}

/** @brief Receives a message
 *  See LibCpp::cFramePort::receive
 * @return Length of received message, 0 if no message is present, -1 in case the client disconnected (only once), -2 if no client is present
 */
int cConnectedClient::receive(char* pMsg, int bufferSize, enBlocking blockingMode) {return 0;}

/** @brief Retrieves the IP address of the remote client represented by this instance. */
stIpAddress cConnectedClient::getIpAddressRemote() {stIpAddress addr; return addr;}

/**
 * @brief Sends a message in case the client is present
 * This is a convenience method.
 * @param ppClient
 * @param message
 * @param messageLength
 * @return
 */
int cConnectedClient::send(cConnectedClient** ppClient, const char* message, int messageLength)
{
    if (!*ppClient) return 0;
    return (*ppClient)->send(message, messageLength);
}

/**
 * @brief Receives messages and deletes the client on disconnection.
 * This is a convenience method.
 * @param ppClient
 * @param message
 * @param messageSize
 * @return Length of received message, 0 if no message is present, -1 in case the client disconnected (only once), -2 if no client is present
 */
int cConnectedClient::receive(cConnectedClient** ppClient, char* message, int messageSize)
{
    if (!*ppClient)
        return -2;
    int len = (*ppClient)->receive(message, messageSize);
    if (len==-1)
    {   // Client disconnection
        delete *ppClient;
        *ppClient = nullptr;
    }
    return len;
}

/**
 * @brief Sends a message in case the client is present
 * This is a convenience method.
 * @param ppClient
 * @param message
 * @param messageLength
 * @return
 */
int cConnectedClient::send(cConnectedClient** ppClient, const uint8_t* message, int messageLength)
{
    return cConnectedClient::send(ppClient, (char*)message, messageLength);
}

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cConnectedClient::getSocketDescriptor()
{
    return 0;
}

/**
 * @brief Sets an external instance to execute send and receive operations.
 * @param pExternalSndRcv
 */
void cConnectedClient::setExternalSndRcv(iExternalSndRcv* pExternalSndRcv)
{
    this->pExternalSndRcv = pExternalSndRcv;
}

/**
 * @brief Receives messages and deletes the client on disconnection.
 * This is a convenience method.
 * @param ppClient
 * @param message
 * @param messageSize
 * @return Length of received message, 0 if no message is present, -1 in case the client disconnected (only once), -2 if no client is present
 */
int cConnectedClient::receive(cConnectedClient** ppClient, uint8_t* message, int messageSize)
{
    return cConnectedClient::receive(ppClient, (char*)message, messageSize);
}

/** @brief Constructor */
cTcpServer::cTcpServer() {}

/** @brief Destructor */
cTcpServer::~cTcpServer() {}

/**
 * @brief Sets the Tcp server into operational mode.
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param family Used only in case an interface name is provided
 * @param linkLocal Used only if family is set to IPv6
 * @param acceptedClients Number of clients allowed to connect with the server (0 = maximum according to system limitation)
 * @return
 */
enIpResult cTcpServer::open(int localPort, std::string localInterfaceOrAddressName, enAddressFamily family, bool linkLocal, int acceptedClients, bool openClients)
{
    return enIpResult_Failure;
}

/**
 *  @brief Prepares closing respectively destruction by initiating the friendly ending of the background task.
 * @return 'true' in case the closing process is finished (polling option before calling 'close' call.)
 */
void cTcpServer::prepareClose() {};

/**
 * @brief Frees the resources and finishes the background task
 * This method  waits until the background task is actually finished which may lead
 * to long execution times. Call LibCpp::cTcpServer::prepareClose to avoid multiple
 * long execution times of 'close'.
 */
void cTcpServer::close() {}


/**
 * @brief Returns a newly accepted client if any.
 * The method returns a pointer to a new instanciated object. It is the responcibility
 * of the process fetching this pointer to sometime (or immediately) delete this object.\n
 * Equals the ip stack call to 'accept'.
 * @return pointer to the LibCpp::cConnectedClient instance newly instanciated by the TCP server.
 */
cConnectedClient* cTcpServer::newClient() {return nullptr;}

/**
 * @brief Returns a newly accepted client if any.
 * The method returns a pointer to a new instanciated object. It is the responcibility
 * of the process fetching this pointer to sometime (or immediately) delete this object.\n
 * Equals the ip stack call to 'accept'.
 * @return pointer to the LibCpp::cConnectedClient instance newly instanciated by the TCP server.
 */
cConnectedClient& cTcpServer::client() {return internalClient;}

/**
 * @brief Deliveres the interface ip address the UDP socket is bound to.
 * @return
 */
stIpAddress cTcpServer::getIpAddressLocal() {stIpAddress addr; return addr;}

/**
 * @brief Deliveres the port the object is listening on.
 * @return
 */
int cTcpServer::getLocalPort()
{
    return 0;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/**
 * @brief Accepts a new client, if provided pointer is free.
 * This is a convenience method.\n
 * Accepts a new client if pClient=nullptr, otherwise deletes new clients.
 * @param ppClient Reference to a cConnectedClient* pointer being controlled by the calling process.
 */
bool cTcpServer::newSingleClient(cConnectedClient** ppClient)
{
    cConnectedClient* pNewClient = newClient();
    if (pNewClient)
    {
        if (*ppClient)
        {
            delete pNewClient;
            return false;
        }
        else
        {
            *ppClient = pNewClient;
            return true;
        }
    }
    return false;
}

/** @} */
