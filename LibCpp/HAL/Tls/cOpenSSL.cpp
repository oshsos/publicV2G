// UTF8 (ü)
/**
\file cTcpClient.cpp

\author Dr. Olaf Simon
\author KEA - Science to Business GmbH - Hochschule Osnabrück
\date   2023-01-11

Published under MIT license (see provided license.txt file)

\addtogroup G_LibCpp_HAL_Tls
@{

\class LibCpp::cOpenSSL
*/

#include <openssl/x509.h>

#include "cOpenSSL.h"
#include "../Tools.h"

using namespace std;
using namespace LibCpp;

/**
 * @brief Constructor
 */
cOpenSSL::cOpenSSL() {}

/**
 * @brief Converts a certificates text (name) entry to a string
 * @param pCertificateName
 * @param nid
 * @param nid_SN
 * @return
 */
string cOpenSSL::certificateNameEntryToString(X509_NAME* pCertificateName, int nid, const char* nid_SN)
{
    string entryString;
    if (!pCertificateName) return entryString;
    int lastpos = -1;
    lastpos = X509_NAME_get_index_by_NID(pCertificateName, nid, lastpos);
    if (lastpos != -1)
    {
        X509_NAME_ENTRY *entry = X509_NAME_get_entry(pCertificateName, lastpos);
        ASN1_STRING *asnContent = X509_NAME_ENTRY_get_data(entry);
        unsigned char* pContent;
        int len = ASN1_STRING_to_UTF8(&pContent, asnContent);
        if (len > 0)
            entryString += stringFormat("%s = %s", nid_SN, pContent);
        OPENSSL_free(pContent);
    }
    return entryString;
}

/**
 * @brief Converts a certificate subject or issuer name to a short string representation
 * @param pCertificateName
 * @return
 */
string cOpenSSL::certificateNameToString(X509_NAME* pCertificateName)
{
    string nameString;
    if (!pCertificateName) return nameString;
    nameString += certificateNameEntryToString(pCertificateName, NID_countryName, SN_countryName) + ", ";
    nameString += certificateNameEntryToString(pCertificateName, NID_organizationName, SN_organizationName) + ", ";
    nameString += certificateNameEntryToString(pCertificateName, NID_commonName, SN_commonName);
    return nameString;
}

/**
 * @brief Deliveres a human readable text information of the certificate
 * @param pCertificate
 * @param shortInfo On true returns just a one line string containing the most important meta data.
 * @return
 */
string cOpenSSL::certificateToString(X509* pCertificate, bool shortInfo)
{
    string certString;

    if (!pCertificate) return certString;
    if (pCertificate)
    {
        if (!shortInfo)
        {
            BIO *bio = BIO_new(BIO_s_mem());
            if (bio)
            {
                char text[4096];
                X509_print_ex(bio, pCertificate, XN_FLAG_COMPAT, X509_FLAG_COMPAT);
                int len = BIO_read(bio, text, 4096);
                certString = string(text, len);
                BIO_free(bio);
            }
            else
                certString = "<conversion failure>";
        }
        else
        {
            // char *line;
            // certString = "Owner (subject): ";
            // line = X509_NAME_oneline(X509_get_subject_name(pCertificate), 0, 0);
            // certString += line;
            // OPENSSL_free(line);
            // certString += "; Issued  by: ";
            // line = X509_NAME_oneline(X509_get_issuer_name(pCertificate), 0, 0);
            // certString += line;
            // OPENSSL_free(line);

            X509_NAME* subject = X509_get_subject_name(pCertificate);               // must not be freed
            certString += "Owner(subject): " + certificateNameToString(subject);
            X509_NAME* issuer = X509_get_issuer_name(pCertificate);                 // must not be freed
            certString += " Issuer: " + certificateNameToString(issuer);

            string time;
            char buffer[5];
            int ptr = 0;
            const ASN1_TIME* notAfter = X509_get0_notAfter(pCertificate);
            buffer[0] = 0;
            if (notAfter->type == V_ASN1_UTCTIME) // 2 digit year
            {
                memcpy(buffer, notAfter->data, 2);
                ptr += 2;
                buffer[ptr] = 0;
            }
            else if (notAfter->type == V_ASN1_GENERALIZEDTIME) // 4 digit year
            {
                memcpy(buffer, notAfter->data, 4);
                ptr += 4;
                buffer[ptr] = 0;
            }
            time += buffer;
            memcpy(buffer, notAfter->data + ptr, 2);
            ptr += 2;
            buffer[ptr] = 0;
            time += "-" + string(buffer);
            memcpy(buffer, notAfter->data + ptr, 2);
            ptr += 2;
            buffer[ptr] = 0;
            time += "-" + string(buffer);
            certString += " Expires: " + time;

            cByteArray finger = cOpenSSL::certificateCalculateFingerprint(pCertificate);
            certString += " Fingerprint: SHA256 " + finger.toString();
        }
    }
    return certString;
}

/**
 * @brief Calculates the fingerprint of the certificate
 * The fingerprint is delivered using the same hash algorithm as specified within the certificate.
 * SHA256 is used as fallback, in special cases SHA512 (see: https://docs.openssl.org/3.0/man3/X509_digest/#synopsis).
 * @param pCertificate
 * @return
 */
cByteArray cOpenSSL::certificateCalculateFingerprint(X509* pCertificate)
{
    cByteArray result;
    if (!pCertificate) return result;
    char digBuffer[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    int res = X509_digest(pCertificate, EVP_sha256(), (unsigned char*)digBuffer, &len);
    if (res)
    {
        result.set(digBuffer, len);
    }
    return result;
}

/**
 * @brief Deliveres the certificate in base 64 encoded PEM format
 * This string corresponds to the .pem file format and includes the header and footer lines.
 * @param pCertificate
 * @return
 */
string cOpenSSL::certificateToPem(X509* pCertificate)
{
    string result;
    if (!pCertificate) return result;
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        int res = PEM_write_bio_X509(bio, pCertificate);
        if (res)
        {
            int size = (int)BIO_number_written(bio);
            cByteArray buffer(size);
            X509_print_ex(bio, pCertificate, XN_FLAG_COMPAT, X509_FLAG_COMPAT);
            if (BIO_read(bio, buffer.data(), buffer.size()))
                result = string(buffer.data());
        }
        BIO_free(bio);
    }
    return result;
}

/**
 * @brief Deliveres the certificate as binary DER format
 * This data corresponds to the binary .der file format.
 * @param pCertificate
 * @return
 */
cByteArray cOpenSSL::certificateToDer(X509* pCertificate)
{
    cByteArray res;
    if (!pCertificate) return res;
    unsigned char *buf = nullptr;

    int len = i2d_X509(pCertificate, &buf);
    if (len > 0)
    {
        res.set((char*)buf, len);
        OPENSSL_free(buf);
    }
    return res;
}

/**
 * @brief Callback method for passphrase entering
 * This function returns the password supplied with the free function parameter 'u'.
 * The function is required for opening of private key files.
 * @param buf
 * @param size
 * @param rwflag
 * @param u
 * @return
 */
int LibCpp::pass_cb(char *buf, int size, int rwflag, void *u)
{
    (void)rwflag;     // is normally zero
    (void)u;          // any parameter transferred by the calling function or method

    if (!u || !buf) return 0;
    char *tmp = (char*)u;

    int len = (int)strlen(tmp);
    if (len > size) len = size;
    memcpy(buf, tmp, len);
    return len;
}

/**
 * @brief Returns the private key of a PEM formatted string
 * The return value points to an allocated memory which must be freed by calling EVP_PKEY_free().
 * @param pCertificate
 * @return Created key or nullptr if unsuccessful
 */
EVP_PKEY* cOpenSSL::privateKeyFromPem(const std::string& keyPem, const std::string& passPhrase)
{
    EVP_PKEY* pkey = nullptr;

    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        if (BIO_write(bio, keyPem.c_str(), (int)keyPem.size()))
        {
            pkey = PEM_read_bio_PrivateKey(bio, NULL, pass_cb, (void*)passPhrase.c_str());
        }
        BIO_free(bio);
    }
    return pkey;
}

/**
 * @brief Returns the private key of a PEM formatted string
 * The return value points to an allocated memory which must be freed by calling EVP_PKEY_free().
 * @param pCertificate
 * @return Created key or nullptr if unsuccessful
 */
EVP_PKEY* cOpenSSL::publicKeyFromPem(const std::string& keyPem)
{
    EVP_PKEY* pkey = nullptr;

    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        if (BIO_write(bio, keyPem.c_str(), (int)keyPem.size()))
        {
            pkey = PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr);
        }
        BIO_free(bio);
    }
    return pkey;
}

/**
 * @brief Returns the base 64 encoded public key in PEM format (including header and footer lines).
 * @return
 */
std::string cOpenSSL::publicKeyToPem(EVP_PKEY* pPublicKey)
{
    cByteArray result;
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        int res = PEM_write_bio_PUBKEY(bio, pPublicKey);
        if (res)
        {
            unsigned int size = (unsigned int)BIO_number_written(bio);
            result.set(size);
            if (BIO_read(bio, result.data(), result.size()))
                return result.asString();
        }
        BIO_free(bio);
    }
    return "";
}

/**
 * @brief Returns the binary array representing the public key in DER format.
 * @return
 */
cByteArray cOpenSSL::publicKeyToDer(EVP_PKEY* pPublicKey)
{
    cByteArray res;
    unsigned char *buf = nullptr;

    int len = i2d_PublicKey(pPublicKey, &buf);
    if (len > 0)
    {
        res.set((char*)buf, len);
        OPENSSL_free(buf);
    }
    return res;
}

/**
 * @brief Returns the base 64 encoded public key in PEM format (including header and footer lines).
 * @return
 */
std::string cOpenSSL::privateKeyToPem(const string& passPhrase, EVP_PKEY* pPrivateKey)
{
    cByteArray result;
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        int res = PEM_write_bio_PrivateKey(bio, pPrivateKey, nullptr, (unsigned char*)passPhrase.c_str(), (int)passPhrase.size(), nullptr, nullptr);
        if (res)
        {
            unsigned int size = (unsigned int)BIO_number_written(bio);
            result.set(size);
            if (BIO_read(bio, result.data(), result.size()))
                return result.asString();
        }
        BIO_free(bio);
    }
    return "";
}

/**
 * @brief Returns an unencrypted binary array representing the public key in DER format.
 * @return
 */
cByteArray cOpenSSL::privateKeyToDer(EVP_PKEY* pPrivateKey)
{
    cByteArray res;
    unsigned char *buf = nullptr;

    int len = i2d_PrivateKey(pPrivateKey, &buf);
    if (len > 0)
    {
        res.set((char*)buf, len);
        OPENSSL_free(buf);
    }
    return res;
}

/**
 * @brief Deliveres the public key in DER fromat contained in a private Key e.g. after key pair generation
 * @param pPrivateKey The key structure containing the private and public key
 * @return
 */
cByteArray cOpenSSL::getPublicKeyDer(EVP_PKEY* pPrivateKey)
{
    cByteArray result;
    size_t len = 0;

    if (EVP_PKEY_get_raw_public_key(pPrivateKey, nullptr, &len) > 0)
    {
        result.set((unsigned int)len);
        if (EVP_PKEY_get_raw_public_key(pPrivateKey, (unsigned char*)result.data(), &len) > 0)
        {
            return result;
        }
    }
    result.clear();
    return result;
}

/**
 * @brief Returns a pointer to a new public key instance identical to the certificates public key.
 * The return value points to an allocated memory which must be freed by calling EVP_PKEY_free().
 * @param pCertificate
 * @return
 */
EVP_PKEY* cOpenSSL::certificatePublicKey(X509* pCertificate)
{
    if (!pCertificate) return nullptr;
    return X509_get_pubkey(pCertificate);
}

/**
 * @brief Verifies the subject certificate against the public key of the issuers public key
 * The verification takes place by decoding the signature of the subject certificate followed
 * by a comparison of the subject certificate hash with the decoding result that shoud carry
 * this hash.
 * @param subject
 * @param issuer
 * @return
 */
bool cOpenSSL::certificateVerify(X509* subject, EVP_PKEY* issuerKey)
{
    if (!subject || !issuerKey) return false;
    int ret = X509_verify(subject, issuerKey);
    EVP_PKEY_free(issuerKey);
    return ret>0;
}

/**
 * @brief Verifies the subject certificate against the public key of the issuer certificate
 * The verification takes place by decoding the signature of the subject certificate followed
 * by a comparison of the subject certificate hash with the decoding result that shoud carry
 * this hash.
 * @param subject
 * @param issuer
 * @return
 */
bool cOpenSSL::certificateVerify(X509* subject, X509* issuer)
{
    if (!subject || !issuer) return false;
    EVP_PKEY *issuerKey = X509_get_pubkey(issuer);
    return certificateVerify(subject, issuerKey);
}

/**
 * @brief Reads the sigature from a certificate as character array
 * The signature is the certificate fingerprint hash, beeing asymmetrically encrypted by the issuer.
 * @param pCertificate
 * @return
 */
cByteArray cOpenSSL::certificateGetSignature(X509* pCertificate)
{
    if (!pCertificate) return cByteArray();
    const ASN1_BIT_STRING* sig;
    const X509_ALGOR* alg;
    X509_get0_signature(&sig, &alg, pCertificate);
    return cByteArray((unsigned int)sig->length, (char*)sig->data);
}

/**
 * @brief Creates an instance from binary DER formatted data.
 * @param certificatePem
 * @return
 */
X509* cOpenSSL::certificateFromDer(cByteArray& certificateDer)
{
    const unsigned char* data = (unsigned char*)certificateDer.data();
    return d2i_X509(nullptr, &data, certificateDer.size());
}

/**
 * @brief Decodes a base 64 PEM formatted certificate to the OpenSSL internal structure
 * This method allocates memory, which needs to be freed using X509_free().
 * @param pCertificate
 * @return
 */
X509* cOpenSSL::certificateFromPem(const string& certificatePem)
{
    X509* result = nullptr;
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio)
    {
        int res = BIO_write(bio, certificatePem.c_str(), (int)certificatePem.size());
        if (res)
        {
            result = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
        }
        BIO_free(bio);
    }
    return result;
}

/**
 * @brief Converts a SSL_ERROR code to a human readable string.
 * @param errorCode
 * @return
 */
string cOpenSSL::getTlsErrorString(int errorCode)
{
    std::string err;
    switch (errorCode)
    {
    case SSL_ERROR_NONE:
        return "The TLS/SSL I/O operation completed.";
        break;

    case SSL_ERROR_ZERO_RETURN:
        return "The TLS/SSL connection has been closed.";
        break;

    case SSL_ERROR_WANT_READ:
        return "The read operation did not complete; "
               "the same TLS/SSL I/O function should be called again later.";
        break;

    case SSL_ERROR_WANT_WRITE:
        return "The write operation did not complete; "
               "the same TLS/SSL I/O function should be called again later.";
        break;

    case SSL_ERROR_WANT_CONNECT:
        return "The connect operation did not complete; "
               "the same TLS/SSL I/O function should be called again later.";
        break;

    case SSL_ERROR_WANT_ACCEPT:
        return "The accept operation did not complete; "
               "the same TLS/SSL I/O function should be called again later.";
        break;

    case SSL_ERROR_WANT_X509_LOOKUP:
        return "The operation did not complete because an application callback set"
               " by SSL_CTX_set_client_cert_cb() has asked to be called again. "
               "The TLS/SSL I/O function should be called again later.";
        break;

    case SSL_ERROR_SYSCALL:
        return "Some I/O error occurred. The OpenSSL error queue may contain"
               " more information on the error.";
        break;

    case SSL_ERROR_SSL:
        {
            char errStr[256];
            err = "Error within the OpenSSL library: ";
            ERR_error_string_n(ERR_get_error(), errStr, 256);
            err += string(errStr);
            return err;
        }
        break;

    default:
        return "<Unknown string conversion>";
        break;
    }
}

/**
 * @brief Allocates memory to store a key using an binary DER format as input.
 * The memory must be freed by calling EVP_PKEY_free()!
 * @param keyDer
 * @param isPrivate As the EVP_PKEY structure may contain a key pair, setting the private key must be indicated.
 * @return
 */
EVP_PKEY* cOpenSSL::keyFromDer(cByteArray& keyDer, bool isPrivate)
{
    const unsigned char* pKeyDer = (unsigned char*)keyDer.data();
    if (!isPrivate)
        return d2i_PublicKey(EVP_PKEY_RSA, nullptr, &pKeyDer, (long)keyDer.size());
    else
        return d2i_AutoPrivateKey(nullptr, &pKeyDer, (long)keyDer.size());
}

/**
 * @brief Generates a key pair (standard: RSA encryption).
 * The return value points to an allocated memory which must be freed by calling EVP_PKEY_free().
 * @return Pointer to the allocated memory, nullptr in case of failure.
 */
EVP_PKEY* cOpenSSL::newKeyPair()
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) return nullptr;

    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY *key = nullptr;
    if (EVP_PKEY_keygen(ctx, &key) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY_CTX_free(ctx);
    return key;
}



/**
 * @brief Encrypts a data array
   - https://github.com/openssl/openssl/blob/master/demos/encrypt/rsa_encrypt.c
 * @param in
 * @param publicKey
 * @return
 */
cByteArray cOpenSSL::encrypt(cByteArray& in, EVP_PKEY* destinationPublicKey)
{
    cByteArray out;
    if (!destinationPublicKey) return out;
    bool ok = false;
    size_t resultLen = 0;
    const char *propq = NULL;
    EVP_PKEY_CTX *ctx = NULL;

    ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, destinationPublicKey, propq);
    if (ctx == NULL) goto cleanup;

    if (EVP_PKEY_encrypt_init_ex(ctx, nullptr) <= 0) goto cleanup;

    // Calculate the size required to hold the encrypted data
    if (EVP_PKEY_encrypt(ctx, NULL, &resultLen, (unsigned char*)in.data(), in.size()) <= 0) goto cleanup;

    out.set((unsigned int)resultLen);

    if (EVP_PKEY_encrypt(ctx, (unsigned char*)out.data(), &resultLen, (unsigned char*)in.data(), in.size()) <= 0) goto cleanup;

    ok = true;

cleanup:
    if (!ok)
        out.clear();
    EVP_PKEY_CTX_free(ctx);
    return out;
}

/**
 * @brief Decrypts a binary data array
 * - https://github.com/openssl/openssl/blob/master/demos/encrypt/rsa_encrypt.c
 * @param in
 * @param privateKey
 * @return
 */
cByteArray cOpenSSL::decrypt(cByteArray& in, EVP_PKEY* privateKey)
{
    cByteArray out;
    if (!privateKey) return out;
    cByteArray intermediate;
    bool ok = false;
    size_t resultLen = 0;
    const char *propq = NULL;
    EVP_PKEY_CTX *ctx = NULL;

    ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, privateKey, propq);
    if (ctx == NULL) goto cleanup;

    if (EVP_PKEY_decrypt_init_ex(ctx, nullptr) <= 0) goto cleanup;

    /* Calculate the size required to hold the decrypted data */
    if (EVP_PKEY_decrypt(ctx, NULL, &resultLen, (unsigned char*)in.data(), in.size()) <= 0) goto cleanup;

    intermediate.set((unsigned int)resultLen);

    if (EVP_PKEY_decrypt(ctx, (unsigned char*)intermediate.data(), &resultLen, (unsigned char*)in.data(), in.size()) <= 0) goto cleanup;

    out.set(intermediate.data(), (unsigned int)resultLen);
    ok = true;

cleanup:
    if (!ok) out.set();
    EVP_PKEY_CTX_free(ctx);
    return out;
}

/**
 * @brief Generates a signed digest from the data array
 * The data is hashed followed by a signing encryption of that hash.
 * @param data
 * @param signerPrivateKey
 * @return
 */
cByteArray cOpenSSL::calculateDigestSignature(cByteArray& data, EVP_PKEY* signerPrivateKey)
{
    cByteArray signature;
    if (!signerPrivateKey) return signature;
    size_t size = 0;
    EVP_MD_CTX* m_SignCtx = EVP_MD_CTX_create();

    if (EVP_DigestSignInit(m_SignCtx,NULL, EVP_sha256(), NULL, signerPrivateKey) <= 0) goto cleanup;
    if (EVP_DigestSignUpdate(m_SignCtx, data.data(), data.size()) <= 0)  goto cleanup;
    if (EVP_DigestSignFinal(m_SignCtx, NULL, &size) <= 0) goto cleanup;
    signature.set((unsigned int)size);
    if (EVP_DigestSignFinal(m_SignCtx, (unsigned char*)signature.data(), &size) <= 0) goto cleanup;
cleanup:
    if (m_SignCtx) EVP_MD_CTX_free(m_SignCtx);
    return signature;
}

/**
 * @brief Verifies the hash of the binary data array against a decrypted signing signature
 * @param data
 * @param signature
 * @param signerPublicKey
 * @return
 */
bool cOpenSSL::verifyDigestSignature(cByteArray& data, cByteArray& signature, EVP_PKEY* signerPublicKey)
{
    bool result = false;
    EVP_MD_CTX* m_VerifyCtx = EVP_MD_CTX_create();

    if (EVP_DigestVerifyInit(m_VerifyCtx,NULL, EVP_sha256(),NULL,signerPublicKey)<=0) goto cleanup;
    if (EVP_DigestVerifyUpdate(m_VerifyCtx, data.data(), data.size()) <= 0) goto cleanup;
    if (EVP_DigestVerifyFinal(m_VerifyCtx, (unsigned char*)signature.data(), signature.size() != 1)) goto cleanup;
    result = true;
cleanup:
    if (m_VerifyCtx) EVP_MD_CTX_free(m_VerifyCtx);
    return result;
}

/** @} */
