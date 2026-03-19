#ifndef CTLSCLIENT_H
#define CTLSCLIENT_H

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "cTls.h"
#include "../cFramePort.h"
#include "../cTcpClient.h"
#include "../cDebug.h"

namespace LibCpp
{


class cTlsClient : public cTcpClient, public iExternalSndRcv
{
public:
    cTlsClient(cTcpClient* pTcpClient, cTls::stCertFiles tlsConfig);
    virtual ~cTlsClient();

    virtual enIpResult  open(int serverPort = LibCpp_cIp_TCP_PORT, std::string serverAddressName = "", int localPort = 0, std::string localInterfaceOrAddressName = "", bool async = false); ///< Sets the Udp socket into operation
    virtual bool        prepareClose();         ///< Prepares for closing by friendly closing the background task.
    virtual void        close();                ///< Frees the resources and finishes the background task

    virtual int         send(const char* pMsg = nullptr, int messageLen = 0, enBlocking blockingMode = enBlocking_BUFFER);             ///< Sends a data frame.
    virtual int         receive(char* pMsg = nullptr, int bufferSize = 0, enBlocking blockingMode = enBlocking_NONE);                  ///< Receives a data frame.
    virtual int         receiveBuffer(char** ppMsg = nullptr, enBlocking blockingMode = enBlocking_NONE);                              ///< Receives a data frame without copying
    virtual void        receiveAcknowledge();                                       ///< Acknowledges a reception of a message by 'receiveBuffer'
    virtual void        operate();                                                  ///< Cyclic operation on callback polling mode usage.
    virtual bool        setCallback(iFramePort* pInstance);                         ///< Sets an instance to be called at message receptions.
    virtual bool        deleteCallback (iFramePort* pInstance = nullptr);           ///< Removes an instance from the callback list.
    virtual bool        setFrameCheckCallback (iFrameCheck* pInstance = nullptr);   ///< Sets an instance to control data framing and encoding.

    virtual bool        isConnected();          ///< Checks for the connection state to the server.
    virtual bool        isOpen();               ///< Checks for the client to be in active operation.
    virtual bool        isClosed();             ///< Checks whether the disconnection is completely processed.

    virtual stIpAddress getIpAddressServer();   ///< Retrieves the IP address of the remote client represented by this instance.

    virtual int         getSocketDescriptor();  ///< Returns the socket file descriptor

//    inline bool         isInitialized() {return pSSL != nullptr;};
//    inline bool         isAccepted() {return isAcceptedFlag;};

    int                 onExternalReceive(char* message, int bufferSize);
    int                 onExternalSend(const char* message, int messageSize);
    bool                onPostConnect();

protected:
    void                free();                 ///< Frees the SSL objects

public:
    cDebug              dbg;
protected:
    cTcpClient*         pTcpClient;

    //bool                isAcceptedFlag;

    SSL_CTX*            pCTXSSL;       ///< SSL Context Data Structure
    SSL*                pSSL;          ///< SSL Data Structure
    //SSL_METHOD*  pMTHDSSL;      ///< used to create an SSL_CTX
    X509*               pPeerCert;     ///< Presented certificated by the client, if any.
    cTls::stCertFiles   tlsConfig;     ///< Tls configuration data to be used
};

}

#endif
