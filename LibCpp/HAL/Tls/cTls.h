// UTF8 (ü) //
/**
\file   cTls.h

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2024-11-20

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Tls
@{

 * \class LibCpp::cTls

 *
 * Secure internet communication (TLS) by using the RSA encryption ecosystem and SHA digests under C++
 * Sichere Internet Kommunikation (TLS) unter Anwendung des RSA Verschlüsselungssystems und SHA Kurzformen unter C++
 *
 * RSA (Rivest–Shamir–Adleman) ist ein asymmetrisches kryptographisches Verfahren, das sowohl zum Verschlüsseln als auch zum digitalen Signieren verwendet werden kann.
 * Alternative: DSA (specified in FIBS), allerdings für Verschlüsseln weniger geeignet
 *
 * Der zuverlässige Weg zu einem Empfänger, setzt die Wahl eines Weges und die Identifizierung des Empfängers voraus.
 * Ein öffentlicher Schlüssel verbunden mit dem zugehörigen Eigentümernamen (Klartext) stellt ein Zertifikat dar. (Besser Schlüsselanhänger / Schlüsselset - gegenüber Schlüsselpaar?)
 *
 * Während meiner eigenen Programmierung sicherer Datenübertragung über das Internet habe ich die Erfahrung gemacht, dass keine verständlichen und für typische Anwendungen
 * ausreichend vollständige Erlätuerungen zur Anwendungen der OpenSSL Bibliothek in C++ vorhanden sind. Nicht umsonst wird die Dokumentation zu OpenSSL als unzureichend
 * beschrieben (Zitat eines Anwenders: woodoo-Funktionen). Der Grund dafür ist die Tatsache, dass die Dokumentation zwar die einzelnen Funktionen ausführlich dokumentiert,
 * jedoch ein Gesamtüberblick über das Zusammenwirken der Funktionen fehlt (Durch den OpenSSL wiki wird das jedoch verbessert!).
 * Bei meinen eigenen Recherchen
 * zu diesem Thema benötigte ich viele Tage, um aus einer Vielzahl von Tutorials und Blogs alle notwendigen Informationen zu sammeln. Schon das Auffinden von verständlichen
 * Tutorials zu Einzelthemen ist zeitraubend, die jeweiligen Tutorials passen selten zusammen und häufig trugen Funktionen unverständliche Eingangsparameter,
 * deren Sinn sich kaum erschloss, d.h. vor allem nicht mit den allgemeinen Erläuterungen der asymmetrischen Kryptography übereinstimmten.
 *
 * cByteArray has (quite) the same functionallity as BUF_MEM alias buf_mem_st from the SSL library and encapsutates an dynamically allocated char buffer, being freed on destruction
 * the class or setting its size to zero.
 *
 * Generelle Einführung in TLS durch asymmetrische Verschlüsselungssysteme
 * - https://hackernoon.com/lang/de/Eine-vereinfachte-Anleitung-zu-TLS-Zertifikaten
 * Comprehensive usage of OpenSSL with C++
 * - https://friendlyuser.github.io/posts/tech/cpp/Using_OpenSSL_in_C++_A_Comprehensive_Guide/
 * Signing with C++
 * - https://eclipsesource.com/blogs/2016/09/07/tutorial-code-signing-and-verification-with-openssl/
 * Description of the OpenSSL API for certificate evaluation
 * - https://zakird.com/2013/10/13/certificate-parsing-with-openssl
 * RSA Encryption algorithm and Signing algorithm
 * - https://cryptobook.nakov.com/digital-signatures/rsa-signatures
 * - https://de.wikipedia.org/wiki/RSA-Kryptosystem
 * Einführung in DSA und generelle Erläuterungen zur Verschlüsselung
 * - https://www.geeksforgeeks.org/digital-signature-algorithm-dsa/
 * OpenSSL overwiew documentation
 * - https://docs.openssl.org/3.0/man3/
 * - https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying
 * Helpful links e.g. for installation are:
 * - https://github.com/embeddedmz/socket-cpp
 * - https://stackoverflow.com/questions/3016956/how-do-i-install-the-openssl-libraries-on-ubuntu
 * - https://medium.com/@swiftscreen/so-installieren-sie-openssl-3-unter-ubuntu-20-04-eine-schritt-f%C3%BCr-schritt-anleitung-355f8e4488f4
 * - https://www.ssldragon.com/de/how-to/openssl/install-openssl-windows/
 *
 * Application description
 *
 * The cTls class is required to use OpenSSL encryption because a global instance is used for initializing
 * and closing the library.\n
 * \n
 * Nevertheless its main focus is to provide easy to use classes for the main encryption tasks of the RSA algorithm.
 * The application user interface is based on three classes representing the public key (LibCpp::cTls::cPublicKey),
 * the private key (LibCpp::cTls::cPrivateKey) and the public certificate (LibCpp::cTls::cCertificate).
 * All data is treated as a binary character array which is encapsulated within the class LibCpp::cByteArray to
 * enable dynamic allocation.\n
 * In order to encrypt for a certain recipient, its public key is required and stored within a 'cPublicKey' instance.
 * Using this instance e.g. called 'recipientKey' you can execute several operations. The relevant for encryption tasks are
 * - Data encrytion
    @code
    encrytedData = recipientKey.encrypt(data);
    @endcode
 * - Data signing
    @code
    dataSignature = recipientKey.calculateDigestSignature(data);
    @endcode
 * If you are the recipient decryption is possible using an appropriate instance 'myKey' of the class LibCpp::cTls::cPrivateKey.
 * - Data decrytion
    @code
    decrytedData = recipientKey.decrypt(data);
    @endcode
 * - Data verification
    @code
    isVerified = recipientKey.verifyDigestSignature(data, dataSignature);
    @endcode
 * The third class represents the certificate generated from and containing those public keys (LibCpp::cTls::cCertificate).
 * - Public key extraction
    @code
    recipientKey = certificate.publicKey();
    @endcode
 * - Signature extraction
    @code
    certSignature = certificate.signature();
    @endcode
 * - Calculate the fingerprint
    @code
    certFingerprint = certificate.fingerprint();
    @endcode
 * - Verification against the signers public key or certificate (also stored within the appropriate class instances)
    @code
    isVerified = certificate.verify(signerCert);
    @endcode
 * Further methods address format conversions to and from the binary DER and base 64 text PEM format, as well as
 * human readable text representations of certificates. The LibCpp::cTls class also provides methods for file reading,
 * file writing required for certificate and key file reading and base 64 conversion. Pleas refer to the specific
 * class documentations.\n
 * In order to set up a TLS client/server communication refer to the classes  LibCpp::cTlsClient (client side) and LibCpp::cTlsConnectedClient (server side).
 *
 *
 * Example
    @code
    // Define the data to transfer and store it within an dynamic allocated character array
    const char* text = "Hallo, das ist mein text!";
    cByteArray aText(strlen(text)+1, text);
    printf("Plain message: %s\n", aText.data());

    // Get the public key of the recipient, which is provided within a certificate
    cTls::cCertificate cert(cTls::readTextFile("certificates/client-cert.pem"));
    cTls::cPublicKey publicKey = cert.publicKey();
    // Generate the encrypted data
    cByteArray encrypted = publicKey.encrypt(aText);
    printf("Encrypted message: %s\n", encrypted.data());

    // The recipient does decrypt with his private key.
    cTls::cPrivateKey privateKey(cTls::readTextFile("certificates/client-private.key"));
    cByteArray decrypted = privateKey.decrypt(encrypted);
    printf("Decrypted message: %s\n\n", decrypted.data());

    // Digest signature
    cByteArray textSignature = privateKey.calculateDigestSignature(aText);
    bool check = publicKey.verifyDigestSignature(aText, textSignature);
    printf("Signature verify: %s\n\n", check ? "True":"False");

    // Test of format conversion
    printf("Generated public key is:\n%s\n\n", publicKey.getPem().c_str());
    cByteArray publicArray = publicKey.getDer();
    printf("DER converted public key is:\n%s\n\n", cTls::cPublicKey(publicArray).getPem().c_str());

    printf("Generated private key is:\n%s\n\n", privateKey.getPem().c_str());
    printf("Original private key is:\n%s\n\n", cTls::readTextFile("certificates/client-private.key").c_str());
    cByteArray privateArray = privateKey.getDer();
    printf("DER converted private key is:\n%s\n\n", cTls::cPrivateKey(privateArray).getPem().c_str());

    printf("Generated cert is:\n%s\n\n", cert.getPem().c_str());
    printf("Original cert is:\n%s\n\n", cTls::readTextFile("certificates/client-cert.pem").c_str());
    cByteArray certArray = cert.getDer();
    printf("DER converted cert is:\n%s\n\n", cTls::cCertificate(certArray).getPem().c_str());
    @endcode
 *
 *
 */

#ifndef CTLS_H
#define CTLS_H

#include <string>
#include <openssl/ssl.h>

#include "../Tools.h"

namespace LibCpp
{

/** @brief Class for object oriented OpenSSL library usage.
This class deliveres an object oriented approach for handling the functions provided
by the LibCpp::cOpenSSL class. Using this class does no more require the usage of
any OpenSSL data types and thus simplifies OpenSSL usage for the RSA encrytion ecosystem. */
class cTls
{
public:
    cTls();                           ///< Constructor
    virtual ~cTls();                  ///< Destructor

    /**
     * @brief This struct contains filenames for certificates and private key files.
     */
    struct stCertFiles
    {
        std::string          m_strSSLCertFile;              ///< public certificate including the public key
        std::string          m_strSSLKeyFile;               ///< private key file (possibly encrypted)
        std::string          m_strCAFile;                   ///< public key file of trusted certificates

        stCertFiles(std::string certFile = "", std::string keyFile = "", std::string caFile = "");                                                      ///< Constructor (parameterless for TLS client)
        static stCertFiles std(std::string certFile = "certs/cert.crt", std::string keyFile = "certs/key.key", std::string caFile = "");                ///< Constructor (parameterless for TLS client/server without authenfification)
        static stCertFiles stdAll(std::string certFile = "certs/cert.crt", std::string keyFile = "certs/key.key", std::string caFile = "certs/ca.crt"); ///< Constructor (parameterless for TLS client/server with authenfification)
    };

    class cPrivateKey;
    class cCertificate;

    /** @brief Represents a public key and encryption methods requiring a public key. */
    class cPublicKey
    {
    public:
        friend class cPrivateKey;
        friend class cCertificate;

        cPublicKey();                                                                           ///< @copybrief LibCpp::cTls::cPublicKey::cPublicKey
        ~cPublicKey();                                                                          ///< @copybrief LibCpp::cTls::cPublicKey::~cPublicKey
        cPublicKey(cByteArray& publicKeyDer);                                                   ///< @copybrief LibCpp::cTls::cPublicKey::cPublicKey(cByteArray&)
        cPublicKey(const std::string& publicKeyPem);                                            ///< @copybrief LibCpp::cTls::cPublicKey::cPublicKey(std::string&)
        cPublicKey(EVP_PKEY* publicKeyOpenSSL);                                                 ///< @copybrief LibCpp::cTls::cPublicKey::cPublicKey(EVP_PKEY*)
        cPublicKey(X509* certificate);                                                          ///< @copybrief LibCpp::cTls::cPublicKey::cPublicKey(X509*)

        bool set(cByteArray& publicKeyDer);                                                     ///< @copybrief LibCpp::cTls::cPublicKey::set(cByteArray&)
        bool set(const std::string& publicKeyPem);                                              ///< @copybrief LibCpp::cTls::cPublicKey::set(std::string&)

        cByteArray getDer();                                                                    ///< @copybrief LibCpp::cTls::cPublicKey::getDer
        std::string getPem();                                                                   ///< @copybrief LibCpp::cTls::cPublicKey::getPem

        cByteArray encrypt(cByteArray& in);                                                     ///< @copybrief LibCpp::cTls::cPublicKey::encrypt
        bool verifyDigestSignature(cByteArray& data, cByteArray& signature);                    ///< @copybrief LibCpp::cTls::cPublicKey::verifyDigestSignature

        const EVP_PKEY* getKey();                                                               ///< @copybrief LibCpp::cTls::cPublicKey::getKey
        void clear();                                                                           ///< @copybrief LibCpp::cTls::cPublicKey::clear

    private:
        EVP_PKEY* pPublicKey;                                                                   ///< Pointer to the allocated public key memory in OpenSSL format.
    };

    /** @brief Represents a private key and encryption methods requiring a private key. */
    class cPrivateKey
    {
    public:
        cPrivateKey();                                                                          ///< @copybrief LibCpp::cTls::cPrivateKey::cPrivateKey
        ~cPrivateKey();                                                                         ///< @copybrief LibCpp::cTls::cPrivateKey::~cPrivateKey
        cPrivateKey(cByteArray& privateKeyDer);                                                 ///< @copybrief LibCpp::cTls::cPrivateKey::cPrivateKey(cByteArray&)
        cPrivateKey(const std::string& privateKeyPem, const std::string& passPhrase = "");      ///< @copybrief LibCpp::cTls::cPrivateKey::cPrivateKey(std::string&,std::string&)
        cPrivateKey(EVP_PKEY* privateKeyOpenSSL);                                               ///< @copybrief LibCpp::cTls::cPrivateKey::cPrivateKey(EVP_PKEY*)

        bool set(cByteArray& privateKeyDer);                                                    ///< @copybrief LibCpp::cTls::cPrivateKey::set(cByteArray&)
        bool set(const std::string& privateKeyPem, const std::string& passPhrase = "");         ///< @copybrief LibCpp::cTls::cPrivateKey::set(std::string&,std::string&)

        cByteArray getDer();                                                                    ///< @copybrief LibCpp::cTls::cPrivateKey::getDer
        std::string getPem(const std::string& passPhrase = "");                                 ///< @copybrief LibCpp::cTls::cPrivateKey::getPem

        cByteArray decrypt(cByteArray& in);                                                     ///< @copybrief LibCpp::cTls::cPrivateKey::decrypt
        cByteArray calculateDigestSignature(cByteArray& data);                                  ///< @copybrief LibCpp::cTls::cPrivateKey::calculateDigestSignature
        static cPrivateKey newKeyPair();                                                        ///< @copybrief LibCpp::cTls::cPrivateKey::newKeyPair
        cPublicKey publicKey();                                                                 ///< @copybrief LibCpp::cTls::cPrivateKey::publicKey

        const EVP_PKEY* getKey();                                                               ///< @copybrief LibCpp::cTls::cPrivateKey::getKey
        void clear();                                                                           ///< @copybrief LibCpp::cTls::cPrivateKey::clear

    private:
        EVP_PKEY* pPrivateKey;                                                                  ///< Pointer to the allocated private key memory in OpenSSL format.

    };

    /** @brief Represents a X509 certificate and its containing private key as well as methods to operate with a certificate. */
    class cCertificate
    {
        friend const EVP_PKEY* cPublicKey::getKey();
    public:
        cCertificate();                                                                         ///< @copybrief LibCpp::cTls::cCertificate::cCertificate
        ~cCertificate();                                                                        ///< @copybrief LibCpp::cTls::cCertificate::cCertificate
        cCertificate(cByteArray& certificateDer);                                               ///< @copybrief LibCpp::cTls::cCertificate::cCertificate(cByteArray&)
        cCertificate(const std::string& certificatePem);                                        ///< @copybrief LibCpp::cTls::cCertificate::cCertificate(std::string&)
        cCertificate(X509* certOpenSSL);                                                        ///< @copybrief LibCpp::cTls::cCertificate::cCertificate(X509*)

        bool        set(cByteArray& certificateDer);                                            ///< @copybrief LibCpp::cTls::cCertificate::set(cByteArray&)
        bool        set(const std::string& certificatePem);                                     ///< @copybrief LibCpp::cTls::cCertificate::set(const std::string&

        cByteArray  getDer();                                                                   ///< @copybrief LibCpp::cTls::cCertificate::getDer
        std::string getPem();                                                                   ///< @copybrief LibCpp::cTls::cCertificate::getPem

        cPublicKey  publicKey();                                                                ///< @copybrief LibCpp::cTls::cCertificate::publicKey
        cByteArray  signature();                                                                ///< @copybrief LibCpp::cTls::cCertificate::signature

        cByteArray  fingerprint();                                                              ///< @copybrief LibCpp::cTls::cCertificate::fingerprint
        bool        verify(cPublicKey& issuer);                                                 ///< @copybrief LibCpp::cTls::cCertificate::verify(cPublicKey&)
        bool        verify(cCertificate& issuer);                                               ///< @copybrief LibCpp::cTls::cCertificate::verify(cCertificate&)

        std::string toString(bool shortInfo = false);                                           ///< @copybrief LibCpp::cTls::cCertificate::toString

        const X509* getCert();                                                                  ///< @copybrief LibCpp::cTls::cCertificate::getCert
        void        clear();                                                                    ///< @copybrief LibCpp::cTls::cCertificate::clear

    private:
        X509*       pCert;                                                                      ///< Pointer to the allocated public key memory in OpenSSL format.

    };

    static std::string  base64Encode(cByteArray& binaryData);                                   ///< @copybrief LibCpp::cTls::base64Encode
    static cByteArray   base64Decode(std::string& b64string);                                   ///< @copybrief LibCpp::cTls::base64Decode

    static cByteArray   readFile(const std::string& fileName);                                  ///< @copybrief LibCpp::cTls::readFile
    static bool         writeFile(const std::string& fileName, cByteArray& data);               ///< @copybrief LibCpp::cTls::writeFile
    static std::string  readTextFile(const std::string& fileName);                              ///< @copybrief LibCpp::cTls::readTextFile
    static bool         writeTextFile(const std::string& fileName, const std::string& text);    ///< @copybrief LibCpp::cTls::writeTextFile
};

}

#endif
/** @} */
