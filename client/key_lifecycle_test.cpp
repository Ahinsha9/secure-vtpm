#include <iostream>
#include "../src/VTPM.h"
#include "VTPMDevice.h"
#include "../driver/vtpm_ioctl.h"

int main()
{
    std::cout << "====================================\n";
    std::cout << "       VTPM KEY LIFECYCLE TEST\n";
    std::cout << "====================================\n";

    /*
     * Connect to Linux VTPM device
     */

    VTPMDevice device;

    if (!device.connectDevice())
    {
        std::cerr
            << "ERROR: Could not connect to VTPM driver.\n";

        return 1;
    }

    /*
     * Initialize VTPM
     */

    VTPM tpm;

    tpm.initialize();

    /*
     * Generate a test key
     */

    std::cout << "\n";
    std::cout
        << "Generating RSA-2048 test key...\n";

    int keyId = tpm.generateKey("RSA");

    if (keyId < 0)
    {
        std::cerr
            << "ERROR: Key generation failed.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "Test key generated.\n";

    std::cout
        << "Key ID: "
        << keyId
        << "\n";

    /*
     * List keys before deletion
     */

    std::cout << "\n";
    std::cout
        << "Keys before deletion:\n";

    tpm.listKeys();

    /*
     * Request DELETE_KEY authorization
     */

    std::cout << "\n";
    std::cout
        << "Requesting DELETE_KEY authorization...\n";

    if (!device.authorizeCommand(
            VTPM_CMD_DELETE_KEY,
            keyId))
    {
        std::cerr
            << "ERROR: DELETE_KEY command rejected.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "DELETE_KEY command authorized by driver.\n";

    /*
     * Delete key
     */

    std::cout << "\n";
    std::cout
        << "Deleting key "
        << keyId
        << "...\n";

    if (!tpm.deleteKey(keyId))
    {
        std::cerr
            << "ERROR: Key deletion failed.\n";

        device.disconnectDevice();

        return 1;
    }

    std::cout
        << "Key deleted successfully.\n";

    /*
     * List keys after deletion
     */

    std::cout << "\n";
    std::cout
        << "Keys after deletion:\n";

    tpm.listKeys();

    /*
     * Disconnect
     */

    device.disconnectDevice();

    std::cout << "\n";
    std::cout
        << "KEY LIFECYCLE TEST COMPLETE.\n";

    return 0;
}
