#include <iostream>
#include <string>
#include <vector>

#include "VTPMDevice.h"
#include "../src/VTPM.h"
#include "../driver/vtpm_ioctl.h"


void showMenu()
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "          SECURE VTPM MENU\n";
    std::cout << "========================================\n";

    std::cout << "1.  Show TPM Status\n";
    std::cout << "2.  Generate RSA Key\n";
    std::cout << "3.  List Stored Keys\n";
    std::cout << "4.  Show PCR Values\n";
    std::cout << "5.  Extend PCR\n";
    std::cout << "6.  Establish PCR Security Policy\n";
    std::cout << "7.  Check PCR Security Policy\n";
    std::cout << "8.  Protected Sign Message\n";
    std::cout << "9.  Driver Status\n";
    std::cout << "10. Exit\n";

    std::cout << "========================================\n";
    std::cout << "Enter your choice: ";
}


int main()
{
    std::cout
        << "========================================\n";

    std::cout
        << "       SECURE VIRTUAL TPM\n";

    std::cout
        << "========================================\n";


    /*
     * --------------------------------------------------
     * Connect to Linux VTPM device
     * --------------------------------------------------
     */

    VTPMDevice device;

    if (!device.connectDevice())
    {
        std::cerr
            << "ERROR: Could not connect to "
            << "/dev/vtpm_secure\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Ping driver
     * --------------------------------------------------
     */

    if (!device.ping())
    {
        std::cerr
            << "ERROR: VTPM driver is not responding.\n";

        return 1;
    }


    /*
     * --------------------------------------------------
     * Show driver status
     * --------------------------------------------------
     */

    device.getStatus();


    /*
     * --------------------------------------------------
     * Initialize VTPM
     * --------------------------------------------------
     */

    VTPM tpm;

    tpm.initialize();


    bool running = true;


    /*
     * --------------------------------------------------
     * Main menu
     * --------------------------------------------------
     */

    while (running)
    {
        showMenu();

        int choice;

        std::cin >> choice;


        /*
         * Clear invalid input.
         */

        if (std::cin.fail())
        {
            std::cin.clear();

            std::cin.ignore(
                10000,
                '\n'
            );

            std::cout
                << "Invalid input.\n";

            continue;
        }


        /*
         * ==================================================
         * OPTION 1
         * Show TPM status
         * ==================================================
         */

        if (choice == 1)
        {
            tpm.showStatus();
        }


        /*
         * ==================================================
         * OPTION 2
         * Generate RSA key
         * ==================================================
         */

        else if (choice == 2)
        {
            std::cout
                << "\nRequesting GENERATE_KEY authorization...\n";

            if (!device.authorizeCommand(
                    VTPM_CMD_GENERATE_KEY))
            {
                std::cout
                    << "Driver rejected key generation.\n";

                continue;
            }


            int keyId =
                tpm.generateKey("RSA");


            if (keyId > 0)
            {
                std::cout
                    << "RSA key generated successfully.\n";

                std::cout
                    << "Key ID: "
                    << keyId
                    << "\n";
            }
            else
            {
                std::cout
                    << "RSA key generation failed.\n";
            }
        }


        /*
         * ==================================================
         * OPTION 3
         * List stored keys
         * ==================================================
         */

        else if (choice == 3)
        {
            std::cout
                << "\nRequesting LIST_KEYS authorization...\n";

            if (!device.authorizeCommand(
                    VTPM_CMD_LIST_KEYS))
            {
                std::cout
                    << "Driver rejected key listing.\n";

                continue;
            }

            tpm.listKeys();
        }


        /*
         * ==================================================
         * OPTION 4
         * Show PCR values
         * ==================================================
         */

        else if (choice == 4)
        {
            std::cout
                << "\nRequesting READ_PCR authorization...\n";

            if (!device.authorizeCommand(
                    VTPM_CMD_READ_PCR,
                    0))
            {
                std::cout
                    << "Driver rejected PCR access.\n";

                continue;
            }

            tpm.showPCRs();
        }


        /*
         * ==================================================
         * OPTION 5
         * Extend PCR
         * ==================================================
         */

        else if (choice == 5)
        {
            int pcrIndex;

            std::string measurement;


            std::cout
                << "\nEnter PCR index (0-7): ";

            std::cin >> pcrIndex;


            std::cin.ignore(
                10000,
                '\n'
            );


            if (pcrIndex < 0 ||
                pcrIndex > 7)
            {
                std::cout
                    << "ERROR: Invalid PCR index.\n";

                continue;
            }


            std::cout
                << "Enter measurement: ";

            std::getline(
                std::cin,
                measurement
            );


            if (measurement.empty())
            {
                std::cout
                    << "ERROR: Measurement cannot be empty.\n";

                continue;
            }


            if (!device.authorizeCommand(
                    VTPM_CMD_EXTEND_PCR,
                    pcrIndex))
            {
                std::cout
                    << "Driver rejected PCR extension.\n";

                continue;
            }


            if (tpm.extendPCR(
                    pcrIndex,
                    measurement))
            {
                std::cout
                    << "PCR extended successfully.\n";
            }
            else
            {
                std::cout
                    << "PCR extension failed.\n";
            }
        }


        /*
         * ==================================================
         * OPTION 6
         * Establish PCR security policy
         * ==================================================
         */

        else if (choice == 6)
        {
            std::cout
                << "\nEstablishing trusted PCR0 state...\n";


            if (tpm.establishPCRPolicy())
            {
                std::cout
                    << "\nPCR security policy established.\n";
            }
            else
            {
                std::cout
                    << "\nFailed to establish PCR policy.\n";
            }
        }


        /*
         * ==================================================
         * OPTION 7
         * Check PCR security policy
         * ==================================================
         */

        else if (choice == 7)
        {
            std::cout
                << "\nChecking PCR security policy...\n";


            if (tpm.isPCRPolicyValid())
            {
                std::cout
                    << "PCR POLICY: VALID\n";

                std::cout
                    << "Current PCR0 matches trusted state.\n";
            }
            else
            {
                std::cout
                    << "PCR POLICY: INVALID\n";

                std::cout
                    << "Current PCR0 does not match "
                    << "trusted state.\n";
            }
        }


        /*
         * ==================================================
         * OPTION 8
         * Protected signing
         * ==================================================
         */

        else if (choice == 8)
        {
            int keyId;

            std::string message;


            std::cout
                << "\nEnter Key ID: ";

            std::cin >> keyId;


            std::cin.ignore(
                10000,
                '\n'
            );


            std::cout
                << "Enter message: ";

            std::getline(
                std::cin,
                message
            );


            if (message.empty())
            {
                std::cout
                    << "ERROR: Message cannot be empty.\n";

                continue;
            }


            /*
             * Ask Linux driver to authorize
             * the SIGN operation.
             */

            std::cout
                << "\nRequesting SIGN authorization...\n";


            if (!device.authorizeCommand(
                    VTPM_CMD_SIGN))
            {
                std::cout
                    << "Driver rejected signing request.\n";

                continue;
            }


            std::vector<unsigned char>
                signature;


            /*
             * VTPM performs PCR policy verification.
             */

            if (tpm.signMessage(
                    keyId,
                    message,
                    signature))
            {
                std::cout
                    << "\n========================================\n";

                std::cout
                    << "PROTECTED SIGNING SUCCESSFUL\n";

                std::cout
                    << "========================================\n";

                std::cout
                    << "Key ID       : "
                    << keyId
                    << "\n";

                std::cout
                    << "Signature Size: "
                    << signature.size()
                    << " bytes\n";

                std::cout
                    << "PCR Policy   : VALID\n";

                std::cout
                    << "Private Key  : PROTECTED\n";
            }
            else
            {
                std::cout
                    << "\n========================================\n";

                std::cout
                    << "PROTECTED SIGNING DENIED\n";

                std::cout
                    << "========================================\n";

                std::cout
                    << "Reason: PCR security policy failed.\n";
            }
        }


        /*
         * ==================================================
         * OPTION 9
         * Driver status
         * ==================================================
         */

        else if (choice == 9)
        {
            device.getStatus();
        }


        /*
         * ==================================================
         * OPTION 10
         * Exit
         * ==================================================
         */

        else if (choice == 10)
        {
            running = false;
        }


        /*
         * ==================================================
         * Invalid option
         * ==================================================
         */

        else
        {
            std::cout
                << "Invalid choice.\n";
        }
    }


    /*
     * --------------------------------------------------
     * Disconnect device
     * --------------------------------------------------
     */

    device.disconnectDevice();


    std::cout
        << "\nSecure VTPM closed.\n";

    return 0;
}
