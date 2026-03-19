#include <thread>
#include <chrono>

#include "cOpenSSL.h"
#include "cTlsClient.h"

using namespace std;
using namespace LibCpp;

/**
 * @brief Constructor
 * @param pTcpClient The hardware dependent TCP client this instance is working with.
 * @param tlsConfig see LibCpp::stTlsConfig.
 */
cTlsClient::cTlsClient(cTcpClient* pTcpClient, cTls::stCertFiles tlsConfig = cTls::stCertFiles()) :
    dbg("cTlsClient"),
    pTcpClient(pTcpClient),
    tlsConfig(tlsConfig)
{
    cDebug dbg("cTlsClient", &this->dbg);

    //isAcceptedFlag = false;
    pCTXSSL = nullptr;
    pSSL = nullptr;
    pPeerCert = nullptr;

    if (pTcpClient->isOpen())
        dbg.printf(enDebugLevel_Fatal, "The client is already opened! Use a TCP client constructed without any parameter.");
}

/**
 * @brief Sets the TLS client into operation
 * @param serverPort
 * @param serverAddressName
 * @param localPort
 * @param localInterfaceOrAddressName
 * @param async
 * @param runTask
 * @return
 */
enIpResult cTlsClient::open(int serverPort, std::string serverAddressName, int localPort, std::string localInterfaceOrAddressName, bool async)
{
    pTcpClient->setExternalSndRcv(this);
    return pTcpClient->open(serverPort, serverAddressName, localPort, localInterfaceOrAddressName, async);
}

/**
 * @brief Callback method being called from the TCP client background task after a connect.
 * @return Server accepted the client.
 * This method handles the TLS handshake between the server and client.
 */
bool cTlsClient::onPostConnect()
{
    cDebug dbg("onPostConnect", &this->dbg);

    bool res = true;

    pCTXSSL = SSL_CTX_new(TLS_client_method());

    if (pCTXSSL)
    {
        /* Load client certificate into the SSL context. */
        if (!tlsConfig.m_strSSLCertFile.empty())
        {
            int err;
            if ((err=SSL_CTX_use_certificate_file(pCTXSSL, tlsConfig.m_strSSLCertFile.c_str(), SSL_FILETYPE_PEM)) <= 0)
            {
                res = false;
                int errCode = SSL_get_error(pSSL, err);
                dbg.printf(enDebugLevel_Error, "Loading client certificate file '%s' failed: %s! Check content of stTlsConfig struct.", tlsConfig.m_strSSLCertFile.c_str(), cOpenSSL::getTlsErrorString(errCode).c_str());
            }
        }
        else
            dbg.printf(enDebugLevel_Error, "No client certificate file is specified. If undesired check the content of the 'stTlsConfig' struct.");

        /* Load the server private-key into the SSL context. */
        if (!tlsConfig.m_strSSLKeyFile.empty())
        {
            int err;
            if ((err=SSL_CTX_use_PrivateKey_file(pCTXSSL, tlsConfig.m_strSSLKeyFile.c_str(), SSL_FILETYPE_PEM)) <= 0)
            {
                res = false;
                int errCode = SSL_get_error(pSSL, err);
                dbg.printf(enDebugLevel_Error, "Loading client private key file '%s' failed: %s! Check the content of the 'stTlsConfig' struct.", tlsConfig.m_strSSLKeyFile.c_str(), cOpenSSL::getTlsErrorString(errCode).c_str());
            }

            // verify private key
            if (!tlsConfig.m_strSSLCertFile.empty())
                if (!SSL_CTX_check_private_key(pCTXSSL))
                {
                    dbg.printf(enDebugLevel_Error, "Server private key does not fit to the server certificate!");
                }
        }
        else
            dbg.printf(enDebugLevel_Error, "No client private key file is specified. If undesired check the content of the 'stTlsConfig' struct.");

        /* Load trusted CA file. */
        if (!tlsConfig.m_strCAFile.empty())
        {
            if (!SSL_CTX_load_verify_locations(pCTXSSL, tlsConfig.m_strCAFile.c_str(), nullptr))
            {
                res = false;
                dbg.printf(enDebugLevel_Error, "Loading trusted CA file '%s' failed! Check the content of the 'stTlsConfig' struct.", tlsConfig.m_strCAFile.c_str());
            }
        }
        else
            dbg.printf(enDebugLevel_Info, "No certificate authorities file is specified. If undesired, check the content of the 'stTlsConfig' struct.");

        /* Set to require peer (client) certificate verification. */
        //SSL_CTX_set_verify(pCTXSSL, SSL_VERIFY_PEER, nullptr);
        //SSL_CTX_set_verify(pCTXSSL, SSL_VERIFY_PEER, VerifyCallback);
        //SSL_CTX_set_verify_depth(pCTXSSL, 1);

        pSSL = SSL_new(pCTXSSL);
        // set the socket directly into the SSL structure
        SSL_set_fd(pSSL, pTcpClient->getSocketDescriptor());
    }
    else
    {
        res = false;
        dbg.printf(enDebugLevel_Error, "Creation of SSL CTX failed!");
    }

    if (!res)
    {
        free();
        return res;
    }

    /* initiate the TLS/SSL handshake with an TLS/SSL server */
    int iResult;
    while ((iResult = SSL_connect(pSSL)) <= 0)
    {
        int errCode = SSL_get_error(pSSL, iResult);
        if (errCode == SSL_ERROR_WANT_READ || errCode == SSL_ERROR_WANT_WRITE)
        {
            this_thread::sleep_for(chrono::milliseconds(1));
            continue;
        }
        dbg.printf(enDebugLevel_Error, "%s", cOpenSSL::getTlsErrorString(errCode).c_str());
        iResult = 0;
        break;
    }

    if (iResult > 0)
    {
        SSL_connect(pSSL);

        /* The data can now be transmitted securely over this connection. */
        dbg.printf(enDebugLevel_Info, "TLS client is accepted by the server with %s encryption.", SSL_get_cipher(pSSL));

        /*if (SSL_get_peer_certificate(m_SSLConnectSocket.m_pSSL) != nullptr)
         {
            if (SSL_get_verify_result(m_SSLConnectSocket.m_pSSL) == X509_V_OK)
            {
               if (m_eSettingsFlags & ENABLE_LOG)
                  m_oLog("client verification with SSL_get_verify_result() succeeded.");
            }
            else
            {
               if (m_eSettingsFlags & ENABLE_LOG)
                  m_oLog("client verification with SSL_get_verify_result() failed.\n");

               return false;
            }
         }
         else if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("the peer certificate was not presented.");*/

        return true;
    }

    dbg.printf(enDebugLevel_Error, "TLS client is not accepted by the server!");
    return false;
}

/**
 * @brief Destructor
 */
cTlsClient::~cTlsClient()
{
    cTlsClient::close();
}

/**
 * @brief Frees the TLS objects
 */
void cTlsClient::free()
{
    if (pSSL != nullptr)
    {
        /* send the close_notify alert to the peer. */
        SSL_shutdown(pSSL);
        SSL_free(pSSL);
        pSSL = nullptr;
    }
    if (pCTXSSL != nullptr)
    {
        SSL_CTX_free(pCTXSSL);
        pCTXSSL = nullptr;
    }
}

int cTlsClient::onExternalReceive(char* message, int bufferSize)
{
    int len;
    if ((len = SSL_read(pSSL, message, bufferSize)) <= 0)
    {
        int errCode = SSL_get_error(pSSL, len);
        if (errCode == SSL_ERROR_WANT_READ)
        {
            return -2;
        }
//        if (errCode == SSL_ERROR_SYSCALL)   // Client disconnected itself
//            return 0;
        dbg.printf(enDebugLevel_Error, "Receiving failed: %s", cOpenSSL::getTlsErrorString(errCode).c_str());
        return 0;
    }
    return len;
}

int cTlsClient::onExternalSend(const char* message, int messageSize)
{
    int nSent;

    nSent = SSL_write(pSSL, message, messageSize);

    if (nSent <= 0)
    {
        int errCode = SSL_get_error(pSSL, nSent);
        if (errCode == SSL_ERROR_WANT_WRITE)
            return 0;
        dbg.printf(enDebugLevel_Error, "Sending failed: %s", cOpenSSL::getTlsErrorString(errCode).c_str());
        return 0;
    }
    return nSent;
}

/**
 * @brief Closes the used recources.
 * This method also closes the underlaying TCP client the instance es linked to by the constructor.
 */
void cTlsClient::close()
{
    pTcpClient->close();
    free();
}

/**
 * @brief Sends a data frame.
 * @param pMsg
 * @param messageLen
 * @param blockingMode
 * @return
 */
int cTlsClient::send(const char* pMsg, int messageLen, enBlocking blockingMode)
{
    return pTcpClient->send(pMsg, messageLen, blockingMode);
}

/**
 * @brief Receives a data frame.
 * @param pMsg
 * @param bufferSize
 * @param blockingMode
 * @return
 */
int cTlsClient::receive(char* pMsg, int bufferSize, enBlocking blockingMode)
{
    return pTcpClient->receive(pMsg, bufferSize, blockingMode);
}

/**
 * @brief Receives a data frame without copying
 * @param ppMsg
 * @param blockingMode
 * @return
 */
int cTlsClient::receiveBuffer(char** ppMsg, enBlocking blockingMode)
{
    return pTcpClient->receiveBuffer(ppMsg, blockingMode);
}

/**
 * @brief Acknowledges a reception of a message by 'receiveBuffer'
 */
void cTlsClient::receiveAcknowledge()
{
    pTcpClient->receiveAcknowledge();
}

/**
 * @brief Cyclic operation on callback polling mode usage.
 */
void cTlsClient::operate()
{
    pTcpClient->operate();
}

/**
 * @brief Sets an instance to be called at message receptions.
 * @param pInstance
 * @return
 */
bool cTlsClient::setCallback(iFramePort* pInstance)
{
    return pTcpClient->setCallback(pInstance);
}

/**
 * @brief Removes an instance from the callback list.
 * @param pInstance
 * @return
 */
bool cTlsClient::deleteCallback (iFramePort* pInstance)
{
    return pTcpClient->deleteCallback(pInstance);
}

/**
 * @brief Sets an instance to control data framing and encoding.
 * @param pInstance
 * @return
 */
bool cTlsClient::setFrameCheckCallback (iFrameCheck* pInstance)
{
    return pTcpClient->setFrameCheckCallback(pInstance);
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * @return
 */
bool cTlsClient::prepareClose()
{
    return pTcpClient->prepareClose();
}

/**
 * @brief Checks whether the client is connected to a server and no disconnection request process is in progress.
 * @return
 */
bool cTlsClient::isConnected()
{
    return pTcpClient->isConnected();
}

/**
 * @brief Checks for the client to be in active operation.
 * @return
 */
bool cTlsClient::isOpen()
{
    return pTcpClient->isOpen();
}

/**
 * @brief Checks whether the client is completely disconnected
 * Completely disconnected means, a possibly running disconnection process has finished.
 * @return
 */
bool cTlsClient::isClosed()
{
    return pTcpClient->isClosed();
}

/**
 * @brief Retrieves the IP address of the remote client represented by this instance.
 * @return
 */
stIpAddress cTlsClient::getIpAddressServer()
{
    return pTcpClient->getIpAddressServer();
}

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cTlsClient:: getSocketDescriptor()
{
    return pTcpClient->getSocketDescriptor();
}
