// UTF8 (ü) //
/**
\file   cOpenSSL.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-11-20

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Tls
@{

\class LibCpp::cOpenSSL

A sample code for using the implemented methods is given as follows.\n
Nevertheless it is recommended to use LibCpp::cTls classes instead.

@code
    #include "cOpenSSL.h"
    #include "cTls.h"

    // Define the data to transfer and store it within an dynamic allocated character array
    const char* text = "Hallo, das ist mein text!";
    cByteArray aText(strlen(text)+1, text);
    printf("Plain message: %s\n", aText.data());

    // Get the public key of the recipient, which is provided within a certificate
    X509* cert = cOpenSSL::certificateFromPem(cTls::readTextFile("certificates/client-cert.pem"));
    EVP_PKEY* publicKey = cOpenSSL::certificateGetPublicKey(cert);
    // Generate the encrypted data
    cByteArray encrypted = cOpenSSL::encrypt(aText, publicKey);
    printf("Encrypted message: %s\n", encrypted.data());

    // The recipient does decrypt with his private key.
    EVP_PKEY* privateKey = cOpenSSL::privateKeyFromPem(cTls::readTextFile("certificates/client-private.key"));
    cByteArray decrypted = cOpenSSL::decrypt(encrypted, privateKey);
    printf("Decrypted message: %s\n\n", decrypted.data());

    // Digest signature
    cByteArray digestSig = cOpenSSL::calculateDigestSignature(aText, privateKey);

    bool check = cOpenSSL::verifyDigestSignature(aText, digestSig, publicKey);
    printf("Signature verify: %s\n\n", check ? "True":"False");

@endcode
**/

#ifndef COPENSSL_H
#define COPENSSL_H

#include <string>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../Tools.h"

namespace LibCpp
{

int pass_cb(char *buf, int size, int rwflag, void *u);                                                              ///< @copybrief LibCpp::pass_cb

/**
 * @brief The cOpenSSL class deliveres interface functions for the OpenSSL library
 * Goal of this class is to provide OpenSSL wrapping functions using the dynamic char array classes std::string and LibCpp::cByteArray.
 * A comparable small function set makes easy use of the OpenSSL library for key and certificate handling,
 * realizing encryption and signing of data arrays with the RSA encryption ecosystem.
 */
class cOpenSSL
{
public:
    cOpenSSL();

    // Encryption
    static EVP_PKEY* newKeyPair();                                                                                  ///< @copybrief LibCpp::cTls::ssl_newKeyPair
    static cByteArray encrypt(cByteArray& in, EVP_PKEY* destinationPublicKey);                                                 ///< @copybrief LibCpp::cTls::encrypt
    static cByteArray decrypt(cByteArray& in, EVP_PKEY* privateKey);                                                ///< @copybrief LibCpp::cTls::decrypt
    static cByteArray calculateDigestSignature(cByteArray& data, EVP_PKEY* privateKey);                             ///< @copybrief LibCpp::cTls::ssl_calculateSignature
    static bool verifyDigestSignature(cByteArray& data, cByteArray& signature, EVP_PKEY* signerPublicKey);          ///< @copybrief LibCpp::cTls::calculateDigestSignature
    static cByteArray certificateCalculateFingerprint(X509* pCertificate);                                          ///< @copybrief LibCpp::cTls::certificateCalculateFingerprint
    static bool certificateVerify(X509* subject, EVP_PKEY* issuerKey);                                              ///< @copybrief LibCpp::cTls::certificateVerify
    static bool certificateVerify(X509* subject, X509* issuer);                                                     ///< @copybrief LibCpp::cTls::certificateVerify

    // Key: Format conversion and data access
    static EVP_PKEY* keyFromDer(cByteArray& keyDer, bool isPrivate = false);                                        ///< @copybrief LibCpp::cTls::keyFromDer
    static EVP_PKEY* publicKeyFromPem(const std::string& keyPem);                                                   ///< @copybrief LibCpp::cTls::privateKeyFromPem
    static EVP_PKEY* privateKeyFromPem(const std::string& keyPem, const std::string& passPhrase = "");              ///< @copybrief LibCpp::cTls::privateKeyFromPem
    static cByteArray publicKeyToDer(EVP_PKEY* pPublicKey);                                                         ///< @copybrief LibCpp::cTls::publicKeyToDer
    static std::string publicKeyToPem(EVP_PKEY* pPublicKey);                                                        ///< @copybrief LibCpp::cTls::publicKeyToPem
    static cByteArray privateKeyToDer(EVP_PKEY* pPrivateKey);                                                       ///< @copybrief LibCpp::cTls::privateKeyToDer
    static std::string privateKeyToPem(const std::string& passPhrase, EVP_PKEY* pPrivateKey);                       ///< @copybrief LibCpp::cTls::privateKeyToPem
    static cByteArray getPublicKeyDer(EVP_PKEY* pPrivateKey);                                                       ///< @copybrief LibCpp::cTls::getPublicKeyDer

    // Certificate: Format conversion and data access
    static X509* certificateFromDer(cByteArray& certificateDer);                                                    ///< @copybrief LibCpp::cTls::certificateFromDer
    static X509* certificateFromPem(const std::string& certificatePem);                                             ///< @copybrief LibCpp::cTls::certificateFromPem
    static std::string certificateToPem(X509* pCertificate);                                                        ///< @copybrief LibCpp::cTls::certificateToPem
    static cByteArray certificateToDer(X509* pCertificate);                                                         ///< @copybrief LibCpp::cTls::certificateToDer
    static std::string certificateToString(X509* pCertificate, bool shortInfo = false);                             ///< @copybrief LibCpp::cTls::certificateToString
    static std::string certificateNameToString(X509_NAME* pCertificateName);                                        ///< @copybrief LibCpp::cTls::certificateNameToString
    static std::string certificateNameEntryToString(X509_NAME* pCertificateName, int nid, const char* nid_SN);      ///< @copybrief LibCpp::cTls::certificateNameEntryToString
    static cByteArray certificateGetSignature(X509* pCertificate);                                                  ///< @copybrief LibCpp::cTls::certificateGetSignature
    static EVP_PKEY* certificatePublicKey(X509* pCertificate);                                                      ///< @copybrief LibCpp::cTls::certificatePublicKey

    // General
    static std::string getTlsErrorString(int errorCode);                                                            ///< @copybrief LibCpp::cTls::getTlsErrorString

    // std::string getPrivateKeyPem(EVP_PKEY)  PEM_write_bio_PKCS8PrivateKey
};

}
#endif
/** @} */
