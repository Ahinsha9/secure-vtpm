#include "VTPMDevice.h"

#include <iostream>
#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../driver/vtpm_ioctl.h"

#define DEVICE_PATH "/dev/vtpm_secure"


// ==================================================
// CONSTRUCTOR
// ==================================================

VTPMDevice::VTPMDevice()
{
    deviceFd = -1;
}


// ==================================================
// DESTRUCTOR
// ==================================================

VTPMDevice::~VTPMDevice()
{
    disconnectDevice();
}


// ==================================================
// CONNECT TO DEVICE
// ==================================================

bool VTPMDevice::connectDevice()
{
    if (deviceFd >= 0)
    {
        return true;
    }

    deviceFd = open(
        DEVICE_PATH,
        O_RDWR
    );

    if (deviceFd < 0)
    {
        std::cerr
            << "ERROR: Could not open "
            << DEVICE_PATH
            << "\n";

        std::cerr
            << "Reason: "
            << std::strerror(errno)
            << "\n";

        return false;
    }

    std::cout
        << "Connected to Virtual TPM driver.\n";

    return true;
}


// ==================================================
// PING DRIVER
// ==================================================

bool VTPMDevice::ping()
{
    if (!isConnected())
    {
        std::cout
            << "ERROR: Device not connected.\n";

        return false;
    }

    int result = ioctl(
        deviceFd,
        VTPM_IOCTL_PING
    );

    if (result < 0)
    {
        std::cerr
            << "ERROR: Driver PING failed.\n";

        return false;
    }

    std::cout
        << "Driver PING successful.\n";

    return true;
}


// ==================================================
// GET DRIVER STATUS
// ==================================================

bool VTPMDevice::getStatus()
{
    if (!isConnected())
    {
        std::cout
            << "ERROR: Device not connected.\n";

        return false;
    }

    struct vtpm_status status = {};

    int result = ioctl(
        deviceFd,
        VTPM_IOCTL_GET_STATUS,
        &status
    );

    if (result < 0)
    {
        std::cerr
            << "ERROR: Could not retrieve "
            << "driver status.\n";

        return false;
    }

    std::cout << "\n";

    std::cout
        << "====================================\n";

    std::cout
        << "        VTPM DRIVER STATUS\n";

    std::cout
        << "====================================\n";

    std::cout
        << "Active         : "
        << status.active
        << "\n";

    std::cout
        << "Driver Version : "
        << status.driver_version
        << "\n";

    return status.active == 1;
}


// ==================================================
// CHECK CONNECTION
// ==================================================

bool VTPMDevice::isConnected() const
{
    return deviceFd >= 0;
}


// ==================================================
// DISCONNECT DEVICE
// ==================================================

void VTPMDevice::disconnectDevice()
{
    if (deviceFd >= 0)
    {
        close(deviceFd);

        deviceFd = -1;

        std::cout
            << "Virtual TPM device disconnected.\n";
    }
}
bool VTPMDevice::authorizeCommand(
    unsigned int commandId,
    int parameter)
{
    if (!isConnected())
    {
        std::cout
            << "ERROR: Device not connected.\n";

        return false;
    }


    struct vtpm_command command = {};

    command.command_id = commandId;

    command.parameter = parameter;

    command.result = -1;


    int result = ioctl(
        deviceFd,
        VTPM_IOCTL_COMMAND,
        &command
    );


    if (result < 0)
    {
        std::cerr
            << "ERROR: VTPM command rejected "
            << "by driver.\n";

        return false;
    }


    if (command.result != 0)
    {
        std::cerr
            << "ERROR: VTPM command was not authorized.\n";

        return false;
    }


    std::cout
        << "VTPM command authorized by driver.\n";


    return true;
}
