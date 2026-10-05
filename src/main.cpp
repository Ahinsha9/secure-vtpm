#include <iostream>

#include "VTPM.h"

int main()
{
    VTPM tpm;

    // Start Virtual TPM
    tpm.initialize();

    // Show TPM status
    tpm.showStatus();

    // Show initial PCR values
    std::cout << "\n";
    std::cout << "INITIAL PCR VALUES\n";

    tpm.showPCRs();

    // Extend PCR0
    std::cout << "\n";
    std::cout << "------------------------------------\n";
    std::cout << "EXTENDING PCR0\n";
    std::cout << "------------------------------------\n";

    tpm.extendPCR(
        0,
        "Operating System Loaded"
    );

    // Show PCR values after extension
    std::cout << "\n";
    std::cout << "PCR VALUES AFTER EXTENSION\n";

    tpm.showPCRs();

    // Extend PCR0 again
    std::cout << "\n";
    std::cout << "------------------------------------\n";
    std::cout << "EXTENDING PCR0 AGAIN\n";
    std::cout << "------------------------------------\n";

    tpm.extendPCR(
        0,
        "Application Started"
    );

    // Show final PCR values
    std::cout << "\n";
    std::cout << "FINAL PCR VALUES\n";

    tpm.showPCRs();

    return 0;
}
