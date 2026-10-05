#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../driver/vtpm_ioctl.h"


#define DEVICE_PATH "/dev/vtpm_secure"


int main()
{
    std::cout
        << "====================================\n";

    std::cout
        << "      VTPM DRIVER TEST\n";

    std::cout
        << "====================================\n";


    /*
     * Open the Virtual TPM device.
     */

    int device =
        open(
            DEVICE_PATH,
            O_RDWR
        );


    if (device < 0)
    {
        std::cerr
            << "ERROR: Could not open "
            << DEVICE_PATH
            << "\n";

        return 1;
    }


    std::cout
        << "Device opened successfully.\n";


    /*
     * Test PING ioctl.
     */

    std::cout
        << "\nSending PING ioctl...\n";


    if (ioctl(
            device,
            VTPM_IOCTL_PING) < 0)
    {
        std::cerr
            << "ERROR: PING ioctl failed.\n";

        close(device);

        return 1;
    }


    std::cout
        << "PING successful.\n";


    /*
     * Test GET_STATUS ioctl.
     */

    struct vtpm_status status;


    std::cout
        << "\nRequesting VTPM status...\n";


    if (ioctl(
            device,
            VTPM_IOCTL_GET_STATUS,
            &status) < 0)
    {
        std::cerr
            << "ERROR: GET_STATUS ioctl failed.\n";

        close(device);

        return 1;
    }


    std::cout
        << "\n";
    std::cout
        << "------------------------------------\n";
    std::cout
        << "        DRIVER STATUS\n";
    std::cout
        << "------------------------------------\n";


    std::cout
        << "VTPM Active     : "
        << status.active
        << "\n";


    std::cout
        << "Driver Version  : "
        << status.driver_version
        << "\n";


    /*
     * Close device.
     */

    close(device);


    std::cout
        << "\nDevice closed successfully.\n";


    return 0;
}
