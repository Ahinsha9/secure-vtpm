#ifndef PCRMANAGER_H
#define PCRMANAGER_H

#include <string>
#include <vector>

class PCRManager
{
private:
    std::vector<std::string> pcrValues;

public:
    PCRManager();

    void initialize();

    void showPCRs();

    bool extendPCR(
        int pcrIndex,
        const std::string& measurement
    );

    std::string getPCRValue(int pcrIndex) const;
};

#endif
