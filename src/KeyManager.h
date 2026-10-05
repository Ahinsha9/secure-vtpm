#ifndef KEYMANAGER_H
#define KEYMANAGER_H

#include <string>
#include <vector>

#include <openssl/evp.h>

struct KeyInfo
{
    int keyId;
    std::string keyType;
    EVP_PKEY* privateKey;
};

class KeyManager
{
private:
    std::vector<KeyInfo> keys;
    int nextKeyId;

    EVP_PKEY* generateRSAKey();

    bool savePrivateKey(
        int keyId,
        EVP_PKEY* privateKey
    );

    EVP_PKEY* loadPrivateKey(
        int keyId
    );

public:
    KeyManager();

    ~KeyManager();

    int generateKey(
        const std::string& keyType
    );

    void listKeys();

    bool deleteKey(
        int keyId
    );

    bool signMessage(
        int keyId,
        const std::string& message,
        std::vector<unsigned char>& signature
    );

    bool verifySignature(
        int keyId,
        const std::string& message,
        const std::vector<unsigned char>& signature
    );

    bool loadStoredKeys();
};

#endif
