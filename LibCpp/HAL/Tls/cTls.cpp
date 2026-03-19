// UTF8 (ü)
/**
\file cTls.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-11

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Tls
@{

\class LibCpp::cTls
*/

#define _CRT_SECURE_NO_WARNINGS

#include <openssl/err.h>
#include <openssl/bio.h>

#include "cTls.h"
#include "../cDebug.h"
#include "cOpenSSL.h"

using namespace std;
using namespace LibCpp;

cTls globalTls;                     ///< Global instance required for OpenSSL initialization and closing

/**
 * @brief Standard constructor with empty content (If unspecified, usable for a TLS client).
 * A client may be initialized without specified files (It will create some internally in such a case.)\n
 * A TLS server requires at least a certificate file and a private key file (see also stCertFiles::std.
 * @param certFile
 * @param keyFile
 * @param caFile
 */
cTls::stCertFiles::stCertFiles(string certFile, string keyFile, string caFile)
{
    m_strSSLCertFile = certFile;
    m_strSSLKeyFile = keyFile;
    m_strCAFile = caFile;
}

/**
 * @brief Initilizes standard certificate and private key files located within the foulder 'certs' (If unspecified, usable for a TLS client or server without autenfification).
 * The two files 'cert.crt', 'key.key' are used as standard names, the 'ca.crt' is left empty. This may be sufficient for a TLS client and server initialization.
 * The standard files and 'ca.crt' are located withing the foulder 'certs'.
 * @param caFile
 * @return
 */
cTls::stCertFiles cTls::stCertFiles::std(string certFile, string keyFile, string caFile)
{
    stCertFiles config;
    config.m_strSSLCertFile = certFile;
    config.m_strSSLKeyFile = keyFile;
    config.m_strCAFile = caFile;
    return config;
}
/**
 * @brief Initilizes standard files for all entries located within the foulder 'certs' (If unspecified, usable for a TLS client or server with autenfification).
 * The two files 'cert.crt', 'key.key' carry standard names, the 'ca.crt' is left empty. This may be sufficient for a TLS client initialization.
 * All three files carry standard names as required for a TLS server initialization.
 * The standard files 'cert.crt', 'key.key' and 'ca.crt' are located withing the foulder 'certs'.
 * @param certFile
 * @param keyFile
 * @param caFile
 * @return
 */
cTls::stCertFiles cTls::stCertFiles::stdAll(string certFile, string keyFile, string caFile)
{
    stCertFiles config;
    config.m_strSSLCertFile = certFile;
    config.m_strSSLKeyFile = keyFile;
    config.m_strCAFile = caFile;
    return config;
}

/**
 * @brief Construts a global instance to open and close the OpenSSL library
 */
cTls::cTls()
{
    cDebug dbg("cTls", enDebugLevel_Info);

    /* Initialize OpenSSL's SSL libraries: load encryption & hash algorithms for SSL */
    SSL_library_init();

    /* Load the error strings for good error reporting */
    SSL_load_error_strings();

    /* Load BIO error strings. */
    //ERR_load_BIO_strings();

    /* Load all available encryption algorithms. */
    OpenSSL_add_all_algorithms();

    dbg.printf(enDebugLevel_Info, "TLS library version: %s", SSLeay_version(SSLEAY_VERSION));
}

/**
 * @brief Destructor
 */
cTls::~cTls()
{
    ERR_free_strings();
    EVP_cleanup();
}

/**
 * @brief Constructs an empty instance.
 */
cTls::cPublicKey::cPublicKey()
{
    pPublicKey = nullptr;
}

/**
 * @brief Destructs and empties the instance.
 */
cTls::cPublicKey::~cPublicKey()
{
    clear();
}

/**
 * @brief Constructs an instance from binary DER formatted data.
 * @param publicKeyDer
 */
cTls::cPublicKey::cPublicKey(cByteArray& publicKeyDer)
{
    pPublicKey = nullptr;
    set(publicKeyDer);
}

/**
 * @brief Constructs an instance from a PEM formatted string.
 * @param publicKeyPem
 */
cTls::cPublicKey::cPublicKey(const string& publicKeyPem)
{
    pPublicKey = nullptr;
    set(publicKeyPem);
}

/**
 * @brief Overtakes the OpenSSL public key instance.
 * This constructor is aimed to interface to OpenSSL functional implementations not covered by the LibCpp::cTsl classes.
 * An instance takes over the responsibility to free the memory 'publicKeyOpenSSL' is pointing to. The pointer must
 * not be used by the application any longer, for reading operations the method LibCpp::cTls::cPublicKey.getKey()
 * should be used instead.\n
 * Be careful using this constructor. Deleting this instance deletes the 'publicKeyOpenSSL' EVP_PKEY memory within the
 * library. Typically the 'publicKeyOpenSSL' instance should be a copy of an OpenSSL memory (with reference counter incremented)
 * e.g. by a ..._get1_... method.
 * @param publicKeyOpenSSL
 */
cTls::cPublicKey::cPublicKey(EVP_PKEY* publicKeyOpenSSL)
{
    pPublicKey = publicKeyOpenSSL;
}

/**
 * @brief Creates an instance from the public key of an OpenSSL X509 certificate.
 * This constructor is aimed to interface to OpenSSL functional implementations not covered by the LibCpp::cTsl classes.
 * @param publicKeyOpenSSL
 */
cTls::cPublicKey::cPublicKey(X509* certificate)
{
    EVP_PKEY* pubKey = X509_get_pubkey(certificate);
    pPublicKey = pubKey;
}

/**
 * @brief Sets the instance from binary DER formatted data.
 * @param publicKeyDer
 * @return
 */
bool cTls::cPublicKey::set(cByteArray& publicKeyDer)
{
    clear();
    pPublicKey = cOpenSSL::keyFromDer(publicKeyDer);
    return pPublicKey != nullptr;
}

/**
 * @brief Sets the instance from a PEM formatted string.
 * @param publicKeyPem
 * @return
 */
bool cTls::cPublicKey::set(const string& publicKeyPem)
{
    clear();
    pPublicKey = cOpenSSL::publicKeyFromPem(publicKeyPem);
    return pPublicKey != nullptr;
}

/**
 * @brief Returns the binary array representing the public key in DER format.
 * @return
 */
cByteArray cTls::cPublicKey::getDer()
{
    return cOpenSSL::publicKeyToDer(pPublicKey);
}

/**
 * @brief Returns the base 64 encoded public key in PEM format (including header and footer lines).
 * @return
 */
string cTls::cPublicKey::getPem()
{
    return cOpenSSL::publicKeyToPem(pPublicKey);
}

/**
 * @brief Encrypts a byte array. Use the destination public key for encryption!
 * @param in
 * @return
 */
cByteArray cTls::cPublicKey::encrypt(cByteArray& in)
{
    return cOpenSSL::encrypt(in, pPublicKey);
}

/**
 * @brief Verifies a byte data array against a signature given for that data array.
 * @param data
 * @param signature
 * @return
 */
bool cTls::cPublicKey::verifyDigestSignature(cByteArray& data, cByteArray& signature)
{
    return cOpenSSL::verifyDigestSignature(data, signature, pPublicKey);
}

/**
 * @brief Returns the private key pointer to in the OpenSSL internal memory struct, this instance is carrying.
 * @return
 */
const EVP_PKEY* cTls::cPublicKey::getKey()
{
    return pPublicKey;
}

/**
 * @brief Frees the instance from the allocated memory.
 */
void cTls::cPublicKey::clear()
{
    if (pPublicKey)
        EVP_PKEY_free(pPublicKey);
}

/**
 * @brief Constructs an empty instance.
 */
cTls::cPrivateKey::cPrivateKey()
{
    pPrivateKey = nullptr;
}

/**
 * @brief Destructs and empties the instance.
 */
cTls::cPrivateKey::~cPrivateKey()
{
    clear();
}

/**
 * @brief Constructs an instance from unencrypted binary DER formatted data.
 * @param publicKeyDer
 */
cTls::cPrivateKey::cPrivateKey(cByteArray& privateKeyDer)
{
    pPrivateKey = nullptr;
    set(privateKeyDer);
}

/**
 * @brief Constructs an instance from a PEM formatted string.
 * @param publicKeyPem
 */
cTls::cPrivateKey::cPrivateKey(const string& privateKeyPem, const string& passPhrase)
{
    pPrivateKey = nullptr;
    set(privateKeyPem, passPhrase);
}

/**
 * @brief Overtakes the OpenSSL private key instance.
 * This constructor is aimed to interface to OpenSSL functional implementations not covered by the LibCpp::cTsl classes.
 * An instance takes over the responsibility to free the memory 'privateKeyOpenSSL' is pointing to. The pointer must
 * not be used by the application any longer, for reading operations the method LibCpp::cTls::cPrivateKey.getKey()
 * should be used instead.
 * @param publicKeyOpenSSL
 */
cTls::cPrivateKey::cPrivateKey(EVP_PKEY* privateKeyOpenSSL)
{
    pPrivateKey = privateKeyOpenSSL;
}

/**
 * @brief Sets the instance from unencrypted binary DER formatted data.
 * @param publicKeyDer
 * @return
 */
bool cTls::cPrivateKey::set(cByteArray& privateKeyDer)
{
    clear();
    pPrivateKey = cOpenSSL::keyFromDer(privateKeyDer, true);
    return pPrivateKey != nullptr;
}

/**
 * @brief Sets the instance from a PEM formatted string.
 * @param publicKeyPem
 * @return
 */
bool cTls::cPrivateKey::set(const string& privateKeyPem, const string& passPhrase)
{
    clear();
    pPrivateKey = cOpenSSL::privateKeyFromPem(privateKeyPem, passPhrase);
    return pPrivateKey != nullptr;
}

/**
 * @brief Returns an unencrypted binary array representing the public key in DER format.
 * @return
 */
cByteArray cTls::cPrivateKey::getDer()
{
    return cOpenSSL::privateKeyToDer(pPrivateKey);
}

/**
 * @brief Returns the base 64 encoded public key in PEM format (including header and footer lines).
 * @return
 */
string cTls::cPrivateKey::getPem(const string& passPhrase)
{
    return cOpenSSL::privateKeyToPem(passPhrase, pPrivateKey);
}

/**
 * @brief Decrypts a byte array.
 * @param in
 * @return
 */
cByteArray cTls::cPrivateKey::decrypt(cByteArray& in)
{
    return cOpenSSL::decrypt(in, pPrivateKey);
}

/**
 * @brief Verifies a byte data array against a signature given for that data array.
 * @param data
 * @param signature
 * @return
 */
cByteArray cTls::cPrivateKey::calculateDigestSignature(cByteArray& data)
{
    return cOpenSSL::calculateDigestSignature(data, pPrivateKey);
}

/**
 * @brief Creates an instance with a new key pair
 * It is possible to extract the public key using the method cTls::cPrivateKey::publicKey .
 */
cTls::cPrivateKey cTls::cPrivateKey::newKeyPair()
{
    cPrivateKey key;
    key.pPrivateKey = cOpenSSL::newKeyPair();
    return  key;
}

/**
 * @brief Returns the public key e.g. after creating a new key pair
 */
cTls::cPublicKey cTls::cPrivateKey::publicKey()
{
    cByteArray keyDer = cOpenSSL::getPublicKeyDer(pPrivateKey);
    return cPublicKey(keyDer);
}

/**
 * @brief Returns the private key pointer to in the OpenSSL internal memory struct, this instance is carrying.
 * @return
 */
const EVP_PKEY* cTls::cPrivateKey::getKey()
{
    return pPrivateKey;
}

/**
 * @brief Frees the instance from the allocated memory.
 */
void cTls::cPrivateKey::clear()
{
    if (pPrivateKey)
        EVP_PKEY_free(pPrivateKey);
}

/**
 * @brief Constructs an empty instance.
 */
cTls::cCertificate::cCertificate()
{
    pCert = nullptr;
}

/**
 * @brief Destructs and empties the instance.
 */
cTls::cCertificate::~cCertificate()
{
    clear();
}

/**
 * @brief Constructs an instance from binary DER formatted data.
 * @param certificateDer
 */
cTls::cCertificate::cCertificate(cByteArray& certificateDer)
{
    pCert = nullptr;
    set(certificateDer);
}

/**
 * @brief Constructs an instance from a PEM formatted string.
 * @param certificatePem
 */
cTls::cCertificate::cCertificate(const string& certificatePem)
{
    pCert = nullptr;
    set(certificatePem);
}

/**
 * @brief Overtakes the OpenSSL certificate instance.
 * This constructor is aimed to interface to OpenSSL functional implementations not covered by the LibCpp::cTsl classes.
 * An instance takes over the responsibility to free the memory 'certOpenSSL' is pointing to. The pointer must
 * not be used by the application any longer, for reading operations the method LibCpp::cTls::cCertificate.getCert()
 * should be used instead.
 * @param publicKeyOpenSSL
 */
cTls::cCertificate::cCertificate(X509* certOpenSSL)
{
    pCert = certOpenSSL;
}

/**
 * @brief Sets the instance from binary DER formatted data.
 * @param certificatePem
 * @return
 */
bool cTls::cCertificate::set(cByteArray& certificateDer)
{
    clear();
    pCert = cOpenSSL::certificateFromDer(certificateDer);
    return pCert != nullptr;
}

/**
 * @brief Sets the instance from a PEM formatted string.
 * @param publicKeyPem
 * @return
 */
bool cTls::cCertificate::set(const string& certificatePem)
{
    clear();
    pCert = cOpenSSL::certificateFromPem(certificatePem);
    return pCert != nullptr;
}

/**
 * @brief Returns the binary array representing the public key in DER format.
 * @return
 */
cByteArray cTls::cCertificate::getDer()
{
    return cOpenSSL::certificateToDer(pCert);
}

/**
 * @brief Returns the base 64 encoded public key in PEM format (including header and footer lines).
 * @return
 */
string cTls::cCertificate::getPem()
{
    return cOpenSSL::certificateToPem(pCert);
}

/**
 * @brief Returns a text representation of the certificate with full or just short content.
 * @param shortInfo Deliveres just company and fingerprint if true
 * @return
 */
string cTls::cCertificate::toString(bool shortInfo)
{
    return cOpenSSL::certificateToString(pCert, shortInfo);
}

cTls::cPublicKey cTls::cCertificate::publicKey()
{
    // cByteArray key = cOpenSSL::publicKeyToDer(cOpenSSL::certificatePublicKey(pCert));
    // return cTls::cPublicKey(key);
    EVP_PKEY* publicKey = cOpenSSL::certificatePublicKey(pCert);
    cPublicKey key;
    key.pPublicKey = publicKey;
    return key;
}

/**
 * @brief Calculates the certificates fingerprint (SHA256 digest value of its content).
 * @return
 */
cByteArray cTls::cCertificate::fingerprint()
{
    return cOpenSSL::certificateCalculateFingerprint(pCert);
}

/**
 * @brief Deliveres the certificates signature (SHA256 digest value of its content signed by the issuer).
 * @return
 */
cByteArray cTls::cCertificate::signature()
{
    return cOpenSSL::certificateGetSignature(pCert);
}

/**
 * @brief Verifies a byte data array against a signature given for that data array.
 * @param data
 * @param signature
 * @return
 */
bool cTls::cCertificate::verify(cPublicKey& issuer)
{
    return cOpenSSL::certificateVerify(pCert, issuer.pPublicKey);
}

/**
 * @brief Verifies a byte data array against a signature given for that data array.
 * @param data
 * @param signature
 * @return
 */
bool cTls::cCertificate::verify(cCertificate& issuer)
{
    cPublicKey key = issuer.publicKey();
    return verify(key);
}

/**
 * @brief Returns the certificate pointer to the OpenSSL internal memory struct, this instance is carrying.
 * @return
 */
const X509* cTls::cCertificate::getCert()
{
    return pCert;
}

/**
 * @brief Frees the instance from the allocated memory.
 */
void cTls::cCertificate::clear()
{
    if (pCert)
        X509_free(pCert);
}

/**
 * @brief Generates the base 64 encoded string of a binary data array
 * @param binaryData
 * @return
 */
string cTls::base64Encode(cByteArray& binaryData)
{
    string result;

    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, binaryData.data(), binaryData.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    result = bufferPtr->data;
    BIO_free_all(bio);
    return result;
}

/**
 * @brief Decodes a base 64 encoded string of a binary data array
 * @param binaryData
 * @return
 */
cByteArray cTls::base64Decode(string& b64string)
{
    // Calculate the required decoding buffer size.
    size_t len = b64string.size();
    size_t padding = 0;

    if (b64string[len-1] == '=' && b64string[len-2] == '=') // last two chars are =
        padding = 2;
    else if (b64string[len-1] == '=')                       // last char is =
        padding = 1;
    size_t resultLen = (len*3)/4 - padding;

    cByteArray result;
    result.set((unsigned int)resultLen);    // allocate the required memory

    BIO *bio, *b64;

    bio = BIO_new_mem_buf(b64string.c_str(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_read(bio, result.data(), result.size());
    BIO_free_all(bio);

    return result;
}

/**
 * @brief Reads a binary file
 * @param fileName
 * @return
 */
cByteArray cTls::readFile(const string& fileName)
{
    cDebug dbg("cTls::readFile");
    cByteArray result;

    FILE* f = fopen(fileName.c_str(), "rb");
    if (f)
    {
        fseek(f, 0L, SEEK_END);
        unsigned int fsize = ftell(f);
        fseek(f, 0L, SEEK_SET);
        result.set(fsize);
        int len = (int)fread(result.data(), result.size(), 1, f);
        if (len<0)
            dbg.printf(enDebugLevel_Error, "Reading file '%s' failed!", fileName.c_str());
        fclose(f);
    }
    else
    {
        dbg.printf(enDebugLevel_Error, "Opening file '%s' failed!", fileName.c_str());
    }
    return result;
}

/**
 * @brief Reads a binary file
 * @param fileName
 * @return
 */
bool cTls::writeFile(const string& fileName, cByteArray& data)
{
    cDebug dbg("cTls::writeFile");
    int len = 0;

    FILE* f = fopen(fileName.c_str(), "wb");
    if (f)
    {
        len = (int)fwrite(data.data(), data.size(), 1, f);
        if (len<=0)
            dbg.printf(enDebugLevel_Error, "Writing to file '%s' failed!", fileName.c_str());
        fclose(f);
    }
    else
    {
        dbg.printf(enDebugLevel_Error, "Opening file '%s' failed!", fileName.c_str());
    }
    return len>0;
}


/**
 * @brief Reads a text file
 * @param fileName
 * @return
 */
string cTls::readTextFile(const string& fileName)
{
    cByteArray buffer = readFile(fileName);
    return string(buffer.data(), buffer.size());
}

/**
 * @brief Writes a text file
 * @param fileName
 * @return
 */
bool cTls::writeTextFile(const string& fileName, const string& data)
{
    cByteArray dataArr((unsigned int)data.size(), (char*)data.c_str());
    return writeFile(fileName, dataArr);
}
/** @} */
