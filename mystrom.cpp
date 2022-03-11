#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define IP_ADDR "0.0.0.0"
#define MYSTROM_PORT "7979"
#define MYSTROM_ANNOUNCEMENT_LEN 8

// MyStromDevice

class MyStromDevice
{
public:
    char* ip;
    char mac[6];
    int type;
    int flags;

    void print() {
        printf("[MyStromDevice] IP: %s:\n", ip);
        printf("                MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        printf("                Device type: %u - Device flags: %02X\n", type, flags);
    }

    void addToDeviceList();
};

std::vector<MyStromDevice> devices;

void MyStromDevice::addToDeviceList()
{
    printf("[MyStromDevice] Adding to devices list... ");
    for (int i = 0; i < devices.size(); i++)
    {
        MyStromDevice other = devices[i];
        if (strcmp(other.mac, mac) == 0)
        {
            printf("Already in list.\n");
            return;
        }
    }
    devices.push_back(*this);
    printf("Added.\n");
}

// Sockets
// https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock

int iResult;

SOCKET ReceiveSocket;

char ReceiveBuffer[MYSTROM_ANNOUNCEMENT_LEN];
int ReceiveBufferLength = MYSTROM_ANNOUNCEMENT_LEN;

sockaddr_in ReceiveSenderAddr;
int ReceiveSenderAddrSize = sizeof(ReceiveSenderAddr);

bool socket_setup()
{
    WSADATA wsaData;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("[MyStromDiscovery] WSAStartup() failed: %d\n", iResult);
        return false;
    }

    // Setup socket
    ReceiveSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(IP_ADDR, MYSTROM_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("[MyStromDiscovery] getaddrinfo() failed: %d\n", iResult);
        WSACleanup();
        return false;
    }

    ReceiveSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ReceiveSocket == INVALID_SOCKET)
    {
        printf("[MyStromDiscovery] socket() failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    iResult = bind(ReceiveSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("[MyStromDiscovery] bind() failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ReceiveSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool socket_receive()
{
    printf("[MyStromDiscovery] Waiting for UDP connection on %s:%s...\n", IP_ADDR, MYSTROM_PORT);

    do
    {
        iResult = recvfrom(ReceiveSocket, ReceiveBuffer, ReceiveBufferLength, 0, (SOCKADDR *)&ReceiveSenderAddr, &ReceiveSenderAddrSize);
        if (iResult == SOCKET_ERROR)
        {
            int lasterror = WSAGetLastError();
            if (lasterror == 10040) {
                printf("[MyStromDiscovery] Received too many bytes... Ignoring...\n");
            } else {
                printf("[MyStromDiscovery] recvfrom() failed with error %d\n", lasterror);
            }
        }

        MyStromDevice device;

        device.ip = inet_ntoa(ReceiveSenderAddr.sin_addr);
        for (int i = 0; i < 6; i++)
        {
            device.mac[i] = ReceiveBuffer[i];
        }
        device.type = ReceiveBuffer[6];
        device.flags = ReceiveBuffer[7];

        device.print();
        device.addToDeviceList();
    } while (true); // TODO: Add end condition

    printf("[MyStromDiscovery] Receiving finished!\n");

    return true;
}

void socket_cleanup()
{
    closesocket(ReceiveSocket);
    WSACleanup();
}

// Interface

void myStromInterface()
{
    printf("[MyStromInterface] Started!\n");
    while (true)
    {
        std::cout << devices[0].ip;
    }
}

// Discovery

void myStromDiscovery() {
    printf("[MyStromDiscovery] Setting up sockets...\n");
    socket_setup();

    printf("[MyStromDiscovery] Checking for MyStrom devices...\n");
    socket_receive();

    printf("[MyStromDiscovery] Running socket cleanup...\n");
    socket_cleanup();

    printf("[MyStromDiscovery] Process finished.\n");
}

// Main

int main() {
    myStromDiscovery();

    // std::thread discoverythread(myStromDiscovery);
    // std::thread interfacethread(myStromInterface);
    
    // interfacethread.join();
    // discoverythread.join(); 
}
