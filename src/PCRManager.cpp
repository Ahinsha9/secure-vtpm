#include "PCRManager.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>

PCRManager::PCRManager()
{
    pcrValues.resize(8);
}

void PCRManager::initialize()
{
    // SHA-256 produces 32 bytes = 64 hexadecimal characters.
    // Initially, every PCR contains 32 zero bytes.

    std::string initialValue(64, '0');

    for (int i = 0; i < 8; i++)
    {
        pcrValues[i] = initialValue;
    }

    std::cout << "PCR Manager initialized.\n";
    std::cout << "PCR count: 8\n";
}

void PCRManager::showPCRs()
{
    std::cout << "\n";
    std::cout << "====================================\n";
    std::cout << "          PCR VALUES\n";
    std::cout << "====================================\n";

    for (int i = 0; i < 8; i++)
    {
        std::cout << "PCR"
                  << i
                  << " : "
                  << pcrValues[i]
                  << "\n";
    }
}

bool PCRManager::extendPCR(
    int pcrIndex,
    const std::string& measurement)
{
    if (pcrIndex < 0 || pcrIndex >= 8)
    {
        std::cout << "ERROR: Invalid PCR index.\n";
        return false;
    }

    /*
     * TPM PCR Extend concept:
     *
     * newPCR = SHA256(oldPCR + measurement)
     */

    std::string data =
        pcrValues[pcrIndex] + measurement;

    unsigned char hash[EVP_MAX_MD_SIZE];

    unsigned int hashLength = 0;

    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if (context == nullptr)
    {
        std::cout
            << "ERROR: Could not create SHA-256 context.\n";

        return false;
    }

    if (EVP_DigestInit_ex(
            context,
            EVP_sha256(),
            nullptr) <= 0)
    {
        std::cout
            << "ERROR: SHA-256 initialization failed.\n";

        EVP_MD_CTX_free(context);

        return false;
    }

    if (EVP_DigestUpdate(
            context,
            data.data(),
            data.size()) <= 0)
    {
        std::cout
            << "ERROR: Could not process PCR data.\n";

        EVP_MD_CTX_free(context);

        return false;
    }

    if (EVP_DigestFinal_ex(
            context,
            hash,
            &hashLength) <= 0)
    {
        std::cout
            << "ERROR: Could not calculate SHA-256 hash.\n";

        EVP_MD_CTX_free(context);

        return false;
    }

    EVP_MD_CTX_free(context);

    std::stringstream result;

    for (unsigned int i = 0; i < hashLength; i++)
    {
        result << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<int>(hash[i]);
    }

    pcrValues[pcrIndex] = result.str();

    std::cout << "\nPCR EXTEND SUCCESSFUL\n";
    std::cout << "PCR Index : " << pcrIndex << "\n";
    std::cout << "New PCR   : "
              << pcrValues[pcrIndex]
              << "\n";

    return true;
}

std::string PCRManager::getPCRValue(int pcrIndex) const
{
    if (pcrIndex < 0 || pcrIndex >= 8)
    {
        return "";
    }

    return pcrValues[pcrIndex];
}
