#define KEYPASSWORD "12345"

#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <thread>

//#include <openssl/bio.h>
//#include <openssl/err.h>
#include <openssl/ssl.h>

#include "cOpenSSL.h"
#include "cTlsConnectedClient.h"

using namespace std;
using namespace LibCpp;

int pcb(char *buf, int size, int rwflag, void *u)
{
    (void)rwflag;
    (void)u;
    const char* n = KEYPASSWORD;
    int len = (int)strlen(KEYPASSWORD);
    strncpy(buf, n, size);
    buf[size - 1] = '\0';
    return len;
}

int helloCb(SSL *s, int *al, void *arg)
{
    (void) al;
    (void) arg;
    X509* pPeerCert = SSL_get_peer_certificate(s);
    return SSL_CLIENT_HELLO_SUCCESS;
}

int certVerification(X509_STORE_CTX* storeCtx, void* u)
{
    (void) u;
    cDebug dbg("cTlsConnectedClient::certVerification");

    X509* peerCert = X509_STORE_CTX_get_current_cert(storeCtx);
    cTls::cCertificate peerCertificate(peerCert);
    dbg.printf(enDebugLevel_Info, "The peer certificate is:\n%s", peerCertificate.toString().c_str());
    return 1;
}

int peerCertVerification(int preverify_ok, X509_STORE_CTX *storeCtx)
{
    cDebug dbg("cTlsConnectedClient::peerCertVerification");

    X509* peerCert = X509_STORE_CTX_get_current_cert(storeCtx);
    cTls::cCertificate peerCertificate(peerCert);
    dbg.printf(enDebugLevel_Info, "The peer certificate is:\n%s", peerCertificate.toString().c_str());
    return preverify_ok;
}

cTlsConnectedClient::cTlsConnectedClient(cConnectedClient* pClient, cTls::stCertFiles tlsConfig) :
    dbg("cTlsConnectedClient"),
    pClient(pClient),
    tlsConfig(tlsConfig)
{
    cDebug dbg("cTlsConnectedClient", &this->dbg);

    isAcceptedFlag = false;
    pCTXSSL = nullptr;
    pSSL = nullptr;
    pPeerCert = nullptr;

    if (pClient->isOpen())
        dbg.printf(enDebugLevel_Fatal, "The connected client is already opened! Call the TCP server constructor or 'open' method with the parameter 'openClients' set to 'false' or set the member 'openClients' to false after the 'open' call.");

    pCTXSSL = SSL_CTX_new(TLS_server_method());

    if (pCTXSSL)
    {
        bool certAvailable = false;

        // Password operation for private key
        SSL_CTX_set_default_passwd_cb(pCTXSSL, pcb);
        // Request and verification of peer certificate
        //SSL_CTX_set_client_hello_cb(pCTXSSL, helloCb, nullptr);
        //SSL_CTX_set_cert_verify_callback(pCTXSSL, certVerification, nullptr);
        //SSL_CTX_set_verify(pCTXSSL, SSL_VERIFY_PEER, peerCertVerification);
        //SSL_CTX_set_verify(pCTXSSL, SSL_VERIFY_PEER, nullptr);
        //SSL_CTX_set_verify(pCTXSSL, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
        //SSL_CTX_set_verify_depth(pCTXSSL, 1);

        /* Load server certificate into the SSL context. */
        if (tlsConfig.m_strSSLCertFile.empty())
            tlsConfig.m_strSSLCertFile = "certs/cert.pem";

        int err;
        if ((err=SSL_CTX_use_certificate_file(pCTXSSL, tlsConfig.m_strSSLCertFile.c_str(), SSL_FILETYPE_PEM)) <= 0)
        {
            int errCode = SSL_get_error(pSSL, err);
            dbg.printf(enDebugLevel_Error, "Loading required server certificate file '%s' failed: %s! Check for 'certs/cert.pem or for the application configuration.", tlsConfig.m_strSSLCertFile.c_str(), cOpenSSL::getTlsErrorString(errCode).c_str());
        }
        else
            certAvailable = true;

        /* Load the server private-key into the SSL context. */
        if (tlsConfig.m_strSSLKeyFile.empty() && certAvailable)
            tlsConfig.m_strSSLKeyFile = "certs/key.pem";
        if (!tlsConfig.m_strSSLKeyFile.empty())
        {
            int err;
            if ((err=SSL_CTX_use_PrivateKey_file(pCTXSSL, tlsConfig.m_strSSLKeyFile.c_str(), SSL_FILETYPE_PEM)) <= 0)
            {
                int errCode = SSL_get_error(pSSL, err);
                dbg.printf(enDebugLevel_Error, "Loading server private key file '%s' failed: %s! Check for 'certs/key.pem or for the application configuration.", tlsConfig.m_strSSLKeyFile.c_str(), cOpenSSL::getTlsErrorString(errCode).c_str());
            }

            // verify private key
            if (!tlsConfig.m_strSSLCertFile.empty())
                if (!SSL_CTX_check_private_key(pCTXSSL))
                {
                    dbg.printf(enDebugLevel_Error, "Server private key does not fit to the server certificate!");
                }
        }
        else
        {
            if (certAvailable)
                dbg.printf(enDebugLevel_Error, "No required server private key file is specified! Check for 'certs/cert.pem or for the application configuration.");
        }

        /* Load trusted CA file. */
        if (!tlsConfig.m_strCAFile.empty())
        {
            if (!SSL_CTX_load_verify_locations(pCTXSSL, tlsConfig.m_strCAFile.c_str(), nullptr))
            {
                dbg.printf(enDebugLevel_Error, "Loading trusted CA file '%s' failed! Check for 'certs/ca.pem or for the application configuration.", tlsConfig.m_strCAFile.c_str());
            }
        }
        else
        {
            if (certAvailable)
                dbg.printf(enDebugLevel_Info, "No certificate authorities file is specified! If undesired, check for 'certs/ca.pem or for the application configuration.");
        }

        pSSL = SSL_new(pCTXSSL);
        // set the socket directly into the SSL structure
        SSL_set_fd(pSSL, pClient->getSocketDescriptor());
    }
    else
        dbg.printf(enDebugLevel_Error, "Creation of SSL CTX failed!");

    pClient->setExternalSndRcv(this);
}

cTlsConnectedClient::~cTlsConnectedClient()
{
    dbg.printf(enDebugLevel_Info, "TLS client at remote address %s disconnected.", this->getIpAddressRemote().toString(true).c_str());
    delete pClient;
    if (pPeerCert)
        X509_free(pPeerCert);
    if (pSSL != nullptr)
    {
        /* send the close_notify alert to the peer. */
        SSL_shutdown(pSSL);
        SSL_free(pSSL);
    }
    if (pCTXSSL != nullptr)
        SSL_CTX_free(pCTXSSL);
}

cTlsConnectedClient* cTlsConnectedClient::newClient(cConnectedClient* pNewTcpClient, cTls::stCertFiles tlsConfig)
{
    cTlsConnectedClient* pTlsConnectedClient = new cTlsConnectedClient(pNewTcpClient, tlsConfig);
    if (pTlsConnectedClient->tlsAccept())
        return pTlsConnectedClient;
    else
    {
        delete pTlsConnectedClient;     // also deletes 'pNewTcpClient'!
        return nullptr;
    }
}

bool cTlsConnectedClient::tlsAccept()
{
    cDebug dbg("tlsAccept", &this->dbg);

    if (!isInitialized())
        return false;

    /* wait for a TLS/SSL client to initiate a TLS/SSL handshake */
    int err = 0;
    int cnt = 50;
    while ((err = SSL_accept(pSSL)) <= 0 && --cnt)
    {
        pPeerCert = SSL_get_peer_certificate(pSSL);
        int errCode = SSL_get_error(pSSL, err);
        if (errCode != SSL_ERROR_WANT_READ && errCode != SSL_ERROR_WANT_WRITE)
        {
            dbg.printf(enDebugLevel_Error, "TLS client at remote address %s refused the acceptance of the server: %s", this->getIpAddressRemote().toString(true).c_str(), cOpenSSL::getTlsErrorString(errCode).c_str());
            // Client certificate reachable at this point?
            // pPeerCert = SSL_get_peer_certificate(pSSL);
            return false;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    if (!cnt)
    {
        dbg.printf(enDebugLevel_Error, "TLS client acceptance timed out!");
        return false;
    }

    dbg.printf(enDebugLevel_Info, "TLS client at remote address %s accepted the server.", this->getIpAddressRemote().toString(true).c_str());
    pClient->open();

    /* The TLS/SSL handshake is successfully completed and  a TLS/SSL connection
     * has been established. Now all reads and writes must use SSL. */
    pPeerCert = SSL_get_peer_certificate(pSSL);

    if (pPeerCert)
    {
        dbg.printf(enDebugLevel_Info, "Client Certificate: %s", cOpenSSL::certificateToString(pPeerCert, true).c_str());
        //printf("%s\n", cTls::getCertificatePem(pPeerCert).c_str());
        //printf("%s\n", cTls::getCertificateRaw(pPeerCert).toString().c_str());
        //printf("Signature: %s\n", cTls::getCertificateSignature(pPeerCert).toString().c_str());
        //printf("Public key: %s\n", cOpenSSL::publicKeyToPem(cOpenSSL::certificatePublicKey(pPeerCert)).c_str());

        string caString = cTls::readTextFile("certificates/ca-cert.pem");
        X509* caCert = cOpenSSL::certificateFromPem(caString);
        bool verification = cOpenSSL::certificateVerify(pPeerCert, caCert);
        if (verification)
            dbg.printf(enDebugLevel_Info, "Certificate verified.");
        else
            dbg.printf(enDebugLevel_Info, "Certificate verification failed!");
    }
    else
        dbg.printf(enDebugLevel_Info, "Client Certificate: No certificate presented by the client.");

    isAcceptedFlag = true;
    return true;
}

int cTlsConnectedClient::onExternalReceive(char* message, int bufferSize)
{
    int len;
    len = SSL_read(pSSL, message, bufferSize);
    if (len <= 0)
    {
        int errCode = SSL_get_error(pSSL, len);
        if (errCode == SSL_ERROR_WANT_READ)
            return -2;
        // if (errCode == SSL_ERROR_SSL)   // Client disconnected itself / unexpected read error
        // {
        //     int stackErr = ERR_GET_REASON(ERR_get_error());
        //     if (stackErr == 294)
        //         return 0;
        // }
        dbg.printf(enDebugLevel_Error, "Receiving failed: %s", cOpenSSL::getTlsErrorString(errCode).c_str());
        return 0;
    }
    return len;
}

int cTlsConnectedClient::onExternalSend(const char* message, int messageSize)
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
 * @brief Sends a data frame.
 * @param pMsg
 * @param messageLen
 * @param blockingMode
 * @return
 */
int cTlsConnectedClient::send(const char* pMsg, int messageLen, enBlocking blockingMode)
{
    return pClient->send(pMsg, messageLen, blockingMode);
}

/**
 * @brief Receives a data frame.
 * @param pMsg
 * @param bufferSize
 * @param blockingMode
 * @return
 */
int cTlsConnectedClient::receive(char* pMsg, int bufferSize, enBlocking blockingMode)
{
    return pClient->receive(pMsg, bufferSize, blockingMode);
}

/**
 * @brief Receives a data frame without copying
 * @param ppMsg
 * @param blockingMode
 * @return
 */
int cTlsConnectedClient::receiveBuffer(char** ppMsg, enBlocking blockingMode)
{
    return pClient->receiveBuffer(ppMsg, blockingMode);
}

/**
 * @brief Acknowledges a reception of a message by 'receiveBuffer'
 */
void cTlsConnectedClient::receiveAcknowledge()
{
    pClient->receiveAcknowledge();
}

/**
 * @brief Cyclic operation on callback polling mode usage.
 */
void cTlsConnectedClient::operate()
{
    pClient->operate();
}

/**
 * @brief Sets an instance to be called at message receptions.
 * @param pInstance
 * @return
 */
bool cTlsConnectedClient::setCallback(iFramePort* pInstance)
{
    return pClient->setCallback(pInstance);
}

/**
 * @brief Removes an instance from the callback list.
 * @param pInstance
 * @return
 */
bool cTlsConnectedClient::deleteCallback (iFramePort* pInstance)
{
    return pClient->deleteCallback(pInstance);
}

/**
 * @brief Sets an instance to control data framing and encoding.
 * @param pInstance
 * @return
 */
bool cTlsConnectedClient::setFrameCheckCallback (iFrameCheck* pInstance)
{
    return pClient->setFrameCheckCallback(pInstance);
}

/**
 * @brief Check for the client to be in active operation
 * @return
 */
bool cTlsConnectedClient::open()
{
    return pClient->open();
}

/**
 * @brief Prepares for closing by friendly closing the background task.
 * @return
 */
bool cTlsConnectedClient::prepareClose()
{
    return pClient->prepareClose();
}

/**
 * @brief Retrieves the IP address of the remote client represented by this instance.
 * @return
 */
stIpAddress cTlsConnectedClient::getIpAddressRemote()
{
    return pClient->getIpAddressRemote();
}

/**
 * @brief Returns the socket file descriptor
 * @return
 */
int cTlsConnectedClient:: getSocketDescriptor()
{
    return pClient->getSocketDescriptor();
}
