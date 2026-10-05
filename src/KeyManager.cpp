#include "KeyManager.h"

#include <iostream>
#include <filesystem>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace fs = std::filesystem;

// Directory where encrypted keys are stored
const std::string KEY_DIRECTORY = "data/keys";

// Password used to encrypt/decrypt stored private keys
// NOTE: For this educational project only.
// A real TPM would NOT hard-code a password like this.
const std::string KEY_PASSWORD =
    "VirtualTPM-Secure-Password-2026";


// ============================================================
// CONSTRUCTOR
// ============================================================

KeyManager::KeyManager()
{
    // First key ID
    nextKeyId = 1001;

    // Load previously stored keys
    loadStoredKeys();
}


// ============================================================
// DESTRUCTOR
// ============================================================

KeyManager::~KeyManager()
{
    // Free all OpenSSL key objects
    for (auto& key : keys)
    {
        if (key.privateKey != nullptr)
        {
            EVP_PKEY_free(key.privateKey);
            key.privateKey = nullptr;
        }
    }

    keys.clear();
}


// ============================================================
// GENERATE RSA-2048 KEY
// ============================================================

EVP_PKEY* KeyManager::generateRSAKey()
{
    EVP_PKEY_CTX* context =
        EVP_PKEY_CTX_new_id(
            EVP_PKEY_RSA,
            nullptr
        );

    if (context == nullptr)
    {
        std::cout
            << "ERROR: Could not create RSA context.\n";

        return nullptr;
    }


    // Initialize RSA key generation
    if (EVP_PKEY_keygen_init(context) <= 0)
    {
        std::cout
            << "ERROR: RSA key generation initialization failed.\n";

        EVP_PKEY_CTX_free(context);

        return nullptr;
    }


    // Set RSA key size to 2048 bits
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(
            context,
            2048) <= 0)
    {
        std::cout
            << "ERROR: Could not set RSA key size.\n";

        EVP_PKEY_CTX_free(context);

        return nullptr;
    }


    // Generate the actual RSA key
    EVP_PKEY* privateKey = nullptr;

    if (EVP_PKEY_keygen(
            context,
            &privateKey) <= 0)
    {
        std::cout
            << "ERROR: RSA key generation failed.\n";

        EVP_PKEY_CTX_free(context);

        return nullptr;
    }


    EVP_PKEY_CTX_free(context);

    return privateKey;
}


// ============================================================
// SAVE ENCRYPTED PRIVATE KEY
// ============================================================

bool KeyManager::savePrivateKey(
    int keyId,
    EVP_PKEY* privateKey)
{
    if (privateKey == nullptr)
    {
        std::cout
            << "ERROR: Invalid private key.\n";

        return false;
    }


    // Create data/keys directory
    try
    {
        fs::create_directories(
            KEY_DIRECTORY
        );
    }
    catch (const fs::filesystem_error& error)
    {
        std::cout
            << "ERROR: Could not create key directory.\n";

        return false;
    }


    // Create filename
    std::string filename =
        KEY_DIRECTORY +
        "/key_" +
        std::to_string(keyId) +
        ".pem";


    // Open file for writing
    FILE* file = fopen(
        filename.c_str(),
        "wb"
    );

    if (file == nullptr)
    {
        std::cout
            << "ERROR: Could not open key file.\n";

        return false;
    }


    /*
     * Write encrypted private key.
     *
     * AES-256-CBC is used to encrypt
     * the private key before storing it.
     */

const unsigned char* password =
    reinterpret_cast<const unsigned char*>(
        KEY_PASSWORD.data()
    );

int passwordLength =
    static_cast<int>(
        KEY_PASSWORD.size()
    );

int result = PEM_write_PrivateKey(
    file,
    privateKey,
    EVP_aes_256_cbc(),
    password,
    passwordLength,
    nullptr,
    nullptr
);

    fclose(file);


    if (result != 1)
    {
        std::cout
            << "ERROR: Could not encrypt private key.\n";

        return false;
    }


    std::cout << "\n";
    std::cout
        << "Private key saved securely.\n";

    std::cout
        << "Storage: "
        << filename
        << "\n";

    std::cout
        << "Encryption: AES-256-CBC\n";


    return true;
}


// ============================================================
// LOAD ENCRYPTED PRIVATE KEY
// ============================================================

EVP_PKEY* KeyManager::loadPrivateKey(
    int keyId)
{
    std::string filename =
        KEY_DIRECTORY +
        "/key_" +
        std::to_string(keyId) +
        ".pem";


    // Open encrypted key file
    FILE* file = fopen(
        filename.c_str(),
        "rb"
    );

    if (file == nullptr)
    {
        return nullptr;
    }


    /*
     * Read and decrypt the private key.
     */

    EVP_PKEY* privateKey =
        PEM_read_PrivateKey(
            file,
            nullptr,
            nullptr,
            const_cast<char*>(
                KEY_PASSWORD.c_str()
            )
        );


    fclose(file);


    return privateKey;
}


// ============================================================
// GENERATE KEY
// ============================================================

int KeyManager::generateKey(
    const std::string& keyType)
{
    // Currently only RSA is supported
    if (keyType != "RSA")
    {
        std::cout
            << "ERROR: Currently only RSA keys "
            << "are supported.\n";

        return -1;
    }


    // Generate RSA-2048 key
    EVP_PKEY* privateKey =
        generateRSAKey();


    if (privateKey == nullptr)
    {
        std::cout
            << "ERROR: RSA key generation failed.\n";

        return -1;
    }


    // Assign new key ID
    int generatedId = nextKeyId;


    // Create KeyInfo object
    KeyInfo newKey;

    newKey.keyId = generatedId;

    newKey.keyType = "RSA";

    newKey.privateKey = privateKey;


    // Store key in memory
    keys.push_back(newKey);


    // Increment ID for next key
    nextKeyId++;


    // Save encrypted private key
    if (!savePrivateKey(
            generatedId,
            privateKey))
    {
        std::cout
            << "WARNING: Key generated but "
            << "could not be saved to disk.\n";
    }


    // Display success information
    std::cout << "\n";
    std::cout
        << "====================================\n";

    std::cout
        << "      KEY GENERATED SUCCESSFULLY\n";

    std::cout
        << "====================================\n";

    std::cout
        << "Key ID        : "
        << generatedId
        << "\n";

    std::cout
        << "Key Type      : RSA\n";

    std::cout
        << "Key Size      : 2048 bits\n";

    std::cout
        << "Private Key   : PROTECTED\n";

    std::cout
        << "Storage       : ENCRYPTED\n";


    return generatedId;
}


// ============================================================
// LIST KEYS
// ============================================================

void KeyManager::listKeys()
{
    std::cout << "\n";
    std::cout
        << "====================================\n";

    std::cout
        << "           STORED KEYS\n";

    std::cout
        << "====================================\n";


    if (keys.empty())
    {
        std::cout
            << "No keys available.\n";

        return;
    }


    for (const auto& key : keys)
    {
        std::cout
            << "Key ID        : "
            << key.keyId
            << "\n";

        std::cout
            << "Key Type      : "
            << key.keyType
            << "\n";

        std::cout
            << "Private Key   : PROTECTED\n";

        std::cout
            << "Storage       : ENCRYPTED\n";

        std::cout
            << "------------------------------------\n";
    }
}


// ============================================================
// DELETE KEY
// ============================================================

bool KeyManager::deleteKey(
    int keyId)
{
    for (auto it = keys.begin();
         it != keys.end();
         ++it)
    {
        if (it->keyId == keyId)
        {
            // Free key from memory
            if (it->privateKey != nullptr)
            {
                EVP_PKEY_free(
                    it->privateKey
                );

                it->privateKey = nullptr;
            }


            // Remove from vector
            keys.erase(it);


            // Delete encrypted file
            std::string filename =
                KEY_DIRECTORY +
                "/key_" +
                std::to_string(keyId) +
                ".pem";


            if (fs::exists(filename))
            {
                if (fs::remove(filename))
                {
                    std::cout
                        << "Encrypted key file deleted.\n";
                }
            }


            std::cout
                << "Key "
                << keyId
                << " deleted successfully.\n";


            return true;
        }
    }


    std::cout
        << "ERROR: Key "
        << keyId
        << " not found.\n";


    return false;
}


// ============================================================
// SIGN MESSAGE
// ============================================================

bool KeyManager::signMessage(
    int keyId,
    const std::string& message,
    std::vector<unsigned char>& signature)
{
    EVP_PKEY* privateKey = nullptr;


    // Find key
    for (const auto& key : keys)
    {
        if (key.keyId == keyId)
        {
            privateKey = key.privateKey;
            break;
        }
    }


    if (privateKey == nullptr)
    {
        std::cout
            << "ERROR: Key "
            << keyId
            << " not found.\n";

        return false;
    }


    // Create signing context
    EVP_MD_CTX* context =
        EVP_MD_CTX_new();


    if (context == nullptr)
    {
        std::cout
            << "ERROR: Could not create "
            << "signing context.\n";

        return false;
    }


    // Initialize SHA-256 + RSA signing
    if (EVP_DigestSignInit(
            context,
            nullptr,
            EVP_sha256(),
            nullptr,
            privateKey) <= 0)
    {
        std::cout
            << "ERROR: Signing initialization failed.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Process message
    if (EVP_DigestSignUpdate(
            context,
            message.data(),
            message.size()) <= 0)
    {
        std::cout
            << "ERROR: Could not process message.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Determine signature size
    size_t signatureLength = 0;

    if (EVP_DigestSignFinal(
            context,
            nullptr,
            &signatureLength) <= 0)
    {
        std::cout
            << "ERROR: Could not determine "
            << "signature size.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Allocate space
    signature.resize(
        signatureLength
    );


    // Generate signature
    if (EVP_DigestSignFinal(
            context,
            signature.data(),
            &signatureLength) <= 0)
    {
        std::cout
            << "ERROR: Signature generation failed.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Resize to actual size
    signature.resize(
        signatureLength
    );


    EVP_MD_CTX_free(context);


    std::cout << "\n";
    std::cout
        << "Message signed successfully.\n";

    std::cout
        << "Key ID: "
        << keyId
        << "\n";

    std::cout
        << "Algorithm: RSA-2048 + SHA-256\n";

    std::cout
        << "Private Key: PROTECTED\n";


    return true;
}


// ============================================================
// VERIFY SIGNATURE
// ============================================================

bool KeyManager::verifySignature(
    int keyId,
    const std::string& message,
    const std::vector<unsigned char>& signature)
{
    EVP_PKEY* publicKey = nullptr;


    // Find key
    for (const auto& key : keys)
    {
        if (key.keyId == keyId)
        {
            publicKey = key.privateKey;
            break;
        }
    }


    if (publicKey == nullptr)
    {
        std::cout
            << "ERROR: Key "
            << keyId
            << " not found.\n";

        return false;
    }


    // Create verification context
    EVP_MD_CTX* context =
        EVP_MD_CTX_new();


    if (context == nullptr)
    {
        std::cout
            << "ERROR: Could not create "
            << "verification context.\n";

        return false;
    }


    // Initialize verification
    if (EVP_DigestVerifyInit(
            context,
            nullptr,
            EVP_sha256(),
            nullptr,
            publicKey) <= 0)
    {
        std::cout
            << "ERROR: Verification initialization failed.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Process message
    if (EVP_DigestVerifyUpdate(
            context,
            message.data(),
            message.size()) <= 0)
    {
        std::cout
            << "ERROR: Could not process message.\n";

        EVP_MD_CTX_free(context);

        return false;
    }


    // Verify signature
    int result =
        EVP_DigestVerifyFinal(
            context,
            signature.data(),
            signature.size()
        );


    EVP_MD_CTX_free(context);


    if (result == 1)
    {
        std::cout << "\n";
        std::cout
            << "Signature verification: SUCCESS\n";

        std::cout
            << "Message is authentic.\n";

        return true;
    }
    else
    {
        std::cout << "\n";
        std::cout
            << "Signature verification: FAILED\n";

        std::cout
            << "Message or signature is invalid.\n";

        return false;
    }
}


// ============================================================
// LOAD ALL STORED KEYS
// ============================================================

bool KeyManager::loadStoredKeys()
{
    try
    {
        // If directory does not exist,
        // there are no stored keys.
        if (!fs::exists(KEY_DIRECTORY))
        {
            return true;
        }


        // Search every file in data/keys
        for (const auto& entry :
             fs::directory_iterator(
                 KEY_DIRECTORY))
        {
            // Ignore directories
            if (!entry.is_regular_file())
            {
                continue;
            }


            std::string filename =
                entry.path().filename().string();


            // We only want files beginning with key_
            if (filename.rfind(
                    "key_",
                    0) != 0)
            {
                continue;
            }


            // We only want .pem files
            if (entry.path().extension()
                    != ".pem")
            {
                continue;
            }


            /*
             * Example:
             *
             * key_1001.pem
             *
             * Remove:
             *
             * "key_"
             * ".pem"
             *
             * Result:
             *
             * "1001"
             */

            std::string idText =
                filename.substr(
                    4,
                    filename.size() - 8
                );


            int keyId =
                std::stoi(idText);


            // Load and decrypt private key
            EVP_PKEY* privateKey =
                loadPrivateKey(
                    keyId
                );


            if (privateKey == nullptr)
            {
                std::cout
                    << "WARNING: Could not load key "
                    << keyId
                    << ".\n";

                continue;
            }


            // Create KeyInfo
            KeyInfo key;

            key.keyId = keyId;

            key.keyType = "RSA";

            key.privateKey = privateKey;


            // Store in memory
            keys.push_back(key);


            // Make sure future IDs are unique
            if (keyId >= nextKeyId)
            {
                nextKeyId =
                    keyId + 1;
            }


            std::cout
                << "Loaded stored key: "
                << keyId
                << "\n";
        }
    }
    catch (const std::exception& error)
    {
        std::cout
            << "ERROR: Could not load stored keys.\n";

        return false;
    }


    return true;
}
