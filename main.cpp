#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include "Utilities.h"
#include "Virtualization.h"
#include "AppParams.h"


int ScanPorts(char**arguments)
{

    //get ip address argument
    char * address = arguments[2];

    //check if valid ip
    if(Utilities::isValidIpAddress(address) == false) {
        return -1;
        //check if domain
        //if not domain exit()
    }

    //check if port range is valid
    char * portRange = arguments[3];

    //get port ranges
    char ** ports = Utilities::SplitByCharacter(portRange, ':');

    int startRange = atoi(ports[0]),
            endRange = atoi(ports[1]);

    delete[](ports);

    //check port range is valid 0-65535 (0xffff)
    if(startRange<0 || startRange > endRange || startRange > USHRT_MAX ||
       endRange < 0 || endRange > USHRT_MAX)
        return INVALID_PORT_RANGE;


    int loops = (endRange - startRange);

    int * successfulPorts = new int[loops];
    memset(successfulPorts, INT32_MAX, loops);

    for(int i = 0; i<loops; i++) {
        int sock = socket(AF_INET6, SOCK_STREAM, 0);

        sockaddr_in sockAddr;

        //increase + set port
        sockAddr.sin_port = htons(i + startRange);

        //convert printable ip to network
        inet_pton(AF_INET6, address, &(sockAddr.sin_addr));

        //attempt to connect
        int connectResult = connect(sock, (sockaddr *) &sockAddr, (socklen_t) (sizeof(sockAddr)));

        //check connection result
        if (connectResult == -1) {
            //failed to connect
            continue;
            //TODO: Display connection error
        }

        //log successful port connections
        successfulPorts[i] = i + startRange;
    }
}

int DetermineInstruction(int argumentCount, char**arguments)
{
    if(argumentCount<=0) return -1;
    char * instruction = arguments[1];

    if(strcmp(instruction, PORT_SCANNER) == 0)
    {
        //invalid
        if(argumentCount < 4)
        {
            std::cout << "Invalid pscan arguments" << std::endl;
            std::cout << "pscan address startPortRange:endPortRange";
            return INVALID_INSTRUCTION_ARGUMENTS;
        }
        return ScanPorts(arguments);

    } else if(strcmp(instruction, VIRTUAL_MACHINE) == 0)
    {
        VirtualMachine * virtualM;
        bool * runVM; *runVM = true;
        virtualM->StartVM(runVM);
    }

    return -1;
}

int main(int arg, char** argv) {
    std::cout << "Hello, World!" << std::endl;

    DetermineInstruction(arg, argv);
    return 0;
}
