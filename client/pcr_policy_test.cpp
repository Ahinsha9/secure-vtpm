#include <iostream>
#include <vector>

#include "../src/VTPM.h"
#include "VTPMDevice.h"
#include "../driver/vtpm_ioctl.h"

int main()
{
    std::cout
        << "====================================\n";

    std::cout
        << "       VTPM PCR POLICY TEST\n";

    std::cout
        << "====================================\n";


    /*
     * --------------------------------------------------
     * Connect to Linux VTPM driver
     * --------------------------------------------------
     */

    VTPMDevice device;

    if (!device.connectDevice())
    {
        std::cerr
            << "ERROR: Could not connect to VTPM driver.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Authorize PCR extension
     * --------------------------------------------------
     */

    std::cout
        << "\nRequesting EXTEND_PCR authorization...\n";

    if (!device.authorizeCommand(
            VTPM_CMD_EXTEND_PCR,
            0))
    {
        std::cerr
            << "ERROR: EXTEND_PCR rejected.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Authorize SIGN
     * --------------------------------------------------
     */

    std::cout
        << "\nRequesting SIGN authorization...\n";

    if (!device.authorizeCommand(
            VTPM_CMD_SIGN))
    {
        std::cerr
            << "ERROR: SIGN rejected.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Initialize VTPM
     * --------------------------------------------------
     */

    VTPM tpm;

    tpm.initialize();


    /*
     * --------------------------------------------------
     * Generate RSA key
     * --------------------------------------------------
     */

    std::cout
        << "\nGenerating RSA-2048 key...\n";

    int keyId =
        tpm.generateKey("RSA");

    if (keyId < 0)
    {
        std::cerr
            << "ERROR: Key generation failed.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Establish trusted PCR state
     * --------------------------------------------------
     */

    std::cout
        << "\nEstablishing trusted PCR state...\n";

    if (!tpm.establishPCRPolicy())
    {
        std::cerr
            << "ERROR: Could not establish PCR policy.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Show PCR state
     * --------------------------------------------------
     */

    std::cout
        << "\nCurrent PCR values:\n";

    tpm.showPCRs();


    /*
     * --------------------------------------------------
     * First signing attempt
     *
     * PCR0 should match the trusted state.
     * --------------------------------------------------
     */

    std::string message =
        "Trusted VTPM Message";

    std::vector<unsigned char> signature;


    std::cout
        << "\nSigning while PCR0 is trusted...\n";

    if (tpm.signMessage(
            keyId,
            message,
            signature))
    {
        std::cout
            << "TRUSTED SIGNING: SUCCESS\n";
    }
    else
    {
        std::cout
            << "TRUSTED SIGNING: FAILED\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Simulate a system measurement change.
     * --------------------------------------------------
     */

    std::cout
        << "\nSimulating system state change...\n";

    if (!tpm.extendPCR(
            0,
            "Unauthorized System Modification"))
    {
        std::cerr
            << "ERROR: PCR extension failed.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Show changed PCR
     * --------------------------------------------------
     */

    std::cout
        << "\nPCR values after system change:\n";

    tpm.showPCRs();


    /*
     * --------------------------------------------------
     * Check PCR policy
     * --------------------------------------------------
     */

    std::cout
        << "\nChecking PCR policy...\n";

    if (tpm.isPCRPolicyValid())
    {
        std::cout
            << "ERROR: PCR policy incorrectly accepted.\n";
    }
    else
    {
        std::cout
            << "PCR policy mismatch detected.\n";
    }


    /*
     * --------------------------------------------------
     * Second signing attempt
     *
     * PCR0 has changed, therefore signing
     * must be denied.
     * --------------------------------------------------
     */

    std::vector<unsigned char>
        blockedSignature;


    std::cout
        << "\nAttempting protected signing after PCR change...\n";

    if (tpm.signMessage(
            keyId,
            message,
            blockedSignature))
    {
        std::cout
            << "ERROR: Signing was incorrectly allowed.\n";
    }
    else
    {
        std::cout
            << "PROTECTED SIGNING BLOCKED.\n";

        std::cout
            << "PCR-based protection working correctly.\n";
    }


    /*
     * --------------------------------------------------
     * Disconnect
     * --------------------------------------------------
     */

    device.disconnectDevice();


    std::cout
        << "\nPCR POLICY TEST COMPLETE.\n";

    return 0;
}
