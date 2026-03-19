// UTF8 (ü) //
/**
\file   cTcpClient.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-01-11
\brief  See cTcpClient.cpp

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL
@{

\class LibCpp::cTcpClient
**/

#ifndef CTCPCLIENT_H
#define CTCPCLIENT_H

#include <string>

#include "cIp.h"
#include "cFramePort.h"

namespace LibCpp
{

/**
*   @brief Abstract interface class for ethernet TCP protocol clients
*/
class cTcpClient : public cFramePort
{
public:
    cTcpClient();                               ///< Constructor.
    virtual ~cTcpClient();                      ///< Destructor.

    virtual enIpResult  open(int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = "", int localPort = 0, std::string localInterfaceOrAddressName = "", bool async = false); ///< Sets the Udp socket into operation
    virtual bool        prepareClose();         ///< Prepares for closing by friendly closing the background task. Returns true in case the process is finished.
    virtual void        close();                ///< Frees the resources and finishes the background task

    virtual bool        isConnected();          ///< Checks for the connection state to the server.
    virtual bool        isOpen();               ///< Checks for the client to be in active operation.
    virtual bool        isClosed();             ///< Checks whether the disconnection is completely processed.

    virtual stIpAddress getIpAddressServer();   ///< Reads the configured destionation ip address.
    virtual int         getSocketDescriptor();  ///> Returns the socket file descriptor

    void                setExternalSndRcv(iExternalSndRcv* pExternalSndRcv);    ///< Sets a callback instance to provide alternative 'send' and 'receive' methods

protected:
    iExternalSndRcv*    pExternalSndRcv;            ///< External class used to execute send and receive operations (required for TLS).
};

}

#endif

/** @} */
