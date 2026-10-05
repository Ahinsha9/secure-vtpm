#include <iostream>

#include "VTPMDevice.h"

#include "../driver/vtpm_ioctl.h"


int main()
{
    std::cout
        << "====================================\n";

    std::cout
        << "     VTPM COMMAND PROTOCOL TEST\n";

    std::cout
        << "====================================\n";


    VTPMDevice device;


    // Connect to driver
    if (!device.connectDevice())
    {
        return 1;
    }


    // Test GENERATE_KEY command
    std::cout
        << "\nTesting GENERATE_KEY command...\n";


    if (device.authorizeCommand(
            VTPM_CMD_GENERATE_KEY))
    {
        std::cout
            << "GENERATE_KEY accepted.\n";
    }
    else
    {
        std::cout
            << "GENERATE_KEY rejected.\n";
    }


    // Test LIST_KEYS command
    std::cout
        << "\nTesting LIST_KEYS command...\n";


    if (device.authorizeCommand(
            VTPM_CMD_LIST_KEYS))
    {
        std::cout
            << "LIST_KEYS accepted.\n";
    }
    else
    {
        std::cout
            << "LIST_KEYS rejected.\n";
    }


    // Test valid PCR
    std::cout
        << "\nTesting EXTEND_PCR on PCR0...\n";


    if (device.authorizeCommand(
            VTPM_CMD_EXTEND_PCR,
            0))
    {
        std::cout
            << "EXTEND_PCR PCR0 accepted.\n";
    }
    else
    {
        std::cout
            << "EXTEND_PCR PCR0 rejected.\n";
    }


    // Test invalid PCR
    std::cout
        << "\nTesting EXTEND_PCR on PCR10...\n";


    if (device.authorizeCommand(
            VTPM_CMD_EXTEND_PCR,
            10))
    {
        std::cout
            << "ERROR: PCR10 was incorrectly accepted.\n";
    }
    else
    {
        std::cout
            << "PCR10 correctly rejected.\n";
    }


    device.disconnectDevice();


    std::cout
        << "\nCommand protocol test complete.\n";


    return 0;
}
