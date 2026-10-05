#include <iostream>
#include <vector>
#include <iomanip>

#include "VTPMDevice.h"
#include "../src/VTPM.h"
#include "../driver/vtpm_ioctl.h"

int main()
{
    std::cout << "====================================\n";
    std::cout << "      VTPM SIGN / VERIFY TEST\n";
    std::cout << "====================================\n";

    /*
     * --------------------------------------------------
     * Connect to Linux VTPM device
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
     * Test SIGN authorization
     * --------------------------------------------------
     */

    std::cout << "\n";
    std::cout << "Requesting SIGN authorization...\n";

    if (!device.authorizeCommand(VTPM_CMD_SIGN))
    {
        std::cerr
            << "ERROR: SIGN command was rejected.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "SIGN command authorized by driver.\n";

    /*
     * --------------------------------------------------
     * Start VTPM
     * --------------------------------------------------
     */

    VTPM tpm;

    tpm.initialize();

    /*
     * --------------------------------------------------
     * Generate RSA key
     * --------------------------------------------------
     */

    std::cout << "\n";
    std::cout << "Generating RSA-2048 key...\n";

    int keyId = tpm.generateKey("RSA");

    if (keyId < 0)
    {
        std::cerr
            << "ERROR: Key generation failed.\n";

        device.disconnectDevice();

        return 1;
    }

    /*
     * --------------------------------------------------
     * Message
     * --------------------------------------------------
     */

    std::string message =
        "Secure VTPM Message";

    std::cout << "\n";
    std::cout << "Message: "
              << message
              << "\n";

    /*
     * --------------------------------------------------
     * Sign message
     * --------------------------------------------------
     */

    std::vector<unsigned char> signature;

    std::cout << "\n";
    std::cout << "Signing message...\n";

    if (!tpm.signMessage(
            keyId,
            message,
            signature))
    {
        std::cerr
            << "ERROR: Signing failed.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "Signature generated successfully.\n";

    std::cout
        << "Signature size: "
        << signature.size()
        << " bytes\n";

    /*
     * --------------------------------------------------
     * Test VERIFY authorization
     * --------------------------------------------------
     */

    std::cout << "\n";
    std::cout
        << "Requesting VERIFY authorization...\n";

    if (!device.authorizeCommand(VTPM_CMD_VERIFY))
    {
        std::cerr
            << "ERROR: VERIFY command was rejected.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "VERIFY command authorized by driver.\n";

    /*
     * --------------------------------------------------
     * Verify original message
     * --------------------------------------------------
     */

    std::cout << "\n";
    std::cout
        << "Verifying original message...\n";

    bool valid = tpm.verifySignature(
        keyId,
        message,
        signature
    );

    if (valid)
    {
        std::cout
            << "SIGNATURE VERIFICATION: SUCCESS\n";
    }
    else
    {
        std::cout
            << "SIGNATURE VERIFICATION: FAILED\n";
    }

    /*
     * --------------------------------------------------
     * Verify modified message
     * --------------------------------------------------
     */

    std::string modifiedMessage =
        "Secure VTPM Message Modified";

    std::cout << "\n";
    std::cout
        << "Verifying modified message...\n";

    bool modifiedValid = tpm.verifySignature(
        keyId,
        modifiedMessage,
        signature
    );

    if (modifiedValid)
    {
        std::cout
            << "ERROR: Modified message was accepted.\n";
    }
    else
    {
        std::cout
            << "Modified message correctly rejected.\n";
    }

    /*
     * --------------------------------------------------
     * Disconnect
     * --------------------------------------------------
     */

    device.disconnectDevice();

    std::cout << "\n";
    std::cout
        << "SIGN / VERIFY test complete.\n";

    return 0;
}
