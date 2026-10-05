#include "VTPM.h"

#include <iostream>

// ============================================================
// Constructor
// ============================================================

VTPM::VTPM()
{
    active = false;

    trustedPCR0 = "";
    pcrPolicySet = false;
}


// ============================================================
// TPM INITIALIZATION
// ============================================================

void VTPM::initialize()
{
    active = true;

    // Initialize PCR registers
    pcrManager.initialize();

    std::cout << "\n=================================\n";
    std::cout << "     Secure Virtual TPM\n";
    std::cout << "=================================\n";
    std::cout << "Virtual TPM initialized successfully.\n";
}

// ============================================================
// TPM STATUS
// ============================================================

bool VTPM::isActive() const
{
    return active;
}


void VTPM::showStatus() const
{
    std::cout << "\n========== VTPM STATUS ==========\n";

    if (active)
        std::cout << "VTPM Status : ACTIVE\n";
    else
        std::cout << "VTPM Status : INACTIVE\n";

    std::cout << "PCR Policy  : ";

    if (pcrPolicySet)
        std::cout << "ESTABLISHED\n";
    else
        std::cout << "NOT ESTABLISHED\n";

    std::cout << "=================================\n";
}


// ============================================================
// KEY MANAGEMENT
// ============================================================

int VTPM::generateKey(const std::string& keyType)
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return -1;
    }

    return keyManager.generateKey(keyType);
}


void VTPM::listKeys()
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return;
    }

    keyManager.listKeys();
}


bool VTPM::deleteKey(int keyId)
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return false;
    }

    return keyManager.deleteKey(keyId);
}


// ============================================================
// PCR MANAGEMENT
// ============================================================

void VTPM::showPCRs() const
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return;
    }

    std::cout << "\n========== PCR VALUES ==========\n";

    for (int i = 0; i < 8; i++)
    {
        std::cout << "PCR" << i << " : "
                  << pcrManager.getPCRValue(i)
                  << "\n";
    }

    std::cout << "================================\n";
}


bool VTPM::extendPCR(
    int index,
    const std::string& measurement
)
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return false;
    }

    if (index < 0 || index >= 8)
    {
        std::cout << "ERROR: Invalid PCR index.\n";
        return false;
    }

    return pcrManager.extendPCR(index, measurement);
}


// ============================================================
// PCR SECURITY POLICY
// ============================================================

bool VTPM::establishPCRPolicy()
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return false;
    }

    // Store current PCR0 as trusted value
    trustedPCR0 = pcrManager.getPCRValue(0);

    pcrPolicySet = true;

    std::cout << "\n=================================\n";
    std::cout << "PCR policy established successfully.\n";
    std::cout << "Protected PCR : PCR0\n";
    std::cout << "Trusted PCR0  : "
              << trustedPCR0
              << "\n";
    std::cout << "=================================\n";

    return true;
}


bool VTPM::isPCRPolicyValid() const
{
    if (!pcrPolicySet)
        return false;

    return pcrManager.getPCRValue(0) == trustedPCR0;
}


// ============================================================
// DIGITAL SIGNING
// ============================================================

bool VTPM::signMessage(
    int keyId,
    const std::string& message,
    std::vector<unsigned char>& signature
)
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return false;
    }

    // --------------------------------------------------------
    // PCR POLICY CHECK
    // --------------------------------------------------------

    if (!pcrPolicySet)
    {
        std::cout << "ERROR: No PCR policy established.\n";
        std::cout << "Protected signing denied.\n";
        return false;
    }

    if (!isPCRPolicyValid())
    {
        std::cout << "ERROR: PCR policy mismatch.\n";
        std::cout << "Protected signing denied.\n";
        return false;
    }

    std::cout << "PCR policy verified.\n";
    std::cout << "Protected signing authorized.\n";

    // --------------------------------------------------------
    // Perform RSA signing
    // --------------------------------------------------------

    return keyManager.signMessage(
        keyId,
        message,
        signature
    );
}


// ============================================================
// SIGNATURE VERIFICATION
// ============================================================

bool VTPM::verifySignature(
    int keyId,
    const std::string& message,
    const std::vector<unsigned char>& signature
)
{
    if (!active)
    {
        std::cout << "ERROR: TPM is not active.\n";
        return false;
    }

    return keyManager.verifySignature(
        keyId,
        message,
        signature
    );
}
