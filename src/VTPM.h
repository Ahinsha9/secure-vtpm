#ifndef VTPM_H
#define VTPM_H

#include <string>
#include <vector>

#include "KeyManager.h"
#include "PCRManager.h"

class VTPM
{
private:
    bool active;

    KeyManager keyManager;
    PCRManager pcrManager;

    // PCR-based security policy
    std::string trustedPCR0;
    bool pcrPolicySet;

public:
    // Constructor
    VTPM();

    // TPM lifecycle
    void initialize();
    bool isActive() const;
    void showStatus() const;

    // Key management
    int generateKey(const std::string& keyType);
    void listKeys();
    bool deleteKey(int keyId);

    // PCR management
    void showPCRs() const;
    bool extendPCR(int index, const std::string& measurement);

    // Cryptographic operations
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

    // PCR security policy
    bool establishPCRPolicy();
    bool isPCRPolicyValid() const;
};

#endif
