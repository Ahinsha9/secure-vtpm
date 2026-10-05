#ifndef VTPMDEVICE_H
#define VTPMDEVICE_H

class VTPMDevice
{
private:
    int deviceFd;

public:
    VTPMDevice();

    ~VTPMDevice();

    bool connectDevice();

    bool ping();

    bool getStatus();

    bool authorizeCommand(
        unsigned int commandId,
        int parameter = 0
    );

    bool isConnected() const;

    void disconnectDevice();
};

#endif
