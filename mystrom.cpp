#include <string>
#include <vector>
#include <iostream>
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
    char* mac[6];
    int type;
    int flags;

    char* ip;
};

std::vector<MyStromDevice> devices;

// Sockets
// https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock

SOCKET ReceiveSocket;

char ReceiveBuffer[MYSTROM_ANNOUNCEMENT_LEN];
int ReceiveBufferLength = MYSTROM_ANNOUNCEMENT_LEN;

sockaddr_in ReceiveSenderAddr;
int ReceiveSenderAddrSize = sizeof(ReceiveSenderAddr);

bool socket_setup()
{
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("[mystrom-utils] WSAStartup() failed: %d\n", iResult);
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
        printf("[mystrom-utils] getaddrinfo() failed: %d\n", iResult);
        WSACleanup();
        return false;
    }

    ReceiveSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ReceiveSocket == INVALID_SOCKET)
    {
        printf("[mystrom-utils] socket() failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    iResult = bind(ReceiveSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("[mystrom-utils] bind() failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ReceiveSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool socket_receive()
{
    printf("[mystrom-utils] Waiting for UDP connection on %s:%s...\n", IP_ADDR, MYSTROM_PORT);

    do
    {
        int iResult = recvfrom(ReceiveSocket, ReceiveBuffer, ReceiveBufferLength, 0, (SOCKADDR *)&ReceiveSenderAddr, &ReceiveSenderAddrSize);
        if (iResult == SOCKET_ERROR)
        {
            printf("[mystrom-utils] recvfrom() failed with error %d\n", WSAGetLastError());
        }

        char *device_ip = inet_ntoa(ReceiveSenderAddr.sin_addr);
        char device_mac[6] = {ReceiveBuffer[0], ReceiveBuffer[1], ReceiveBuffer[2], ReceiveBuffer[3], ReceiveBuffer[4], ReceiveBuffer[5]};
        char device_type = ReceiveBuffer[6];
        int device_flags = ReceiveBuffer[7];

        printf("[mystrom-utils] Received from %s:\n", device_ip);
        printf("[mystrom-utils]     MAC in hex: %02X:%02X:%02X:%02X:%02X:%02X\n", device_mac[0], device_mac[1], device_mac[2], device_mac[3], device_mac[4], device_mac[5]);
        printf("[mystrom-utils]     MAC in dec: %u:%u:%u:%u:%u:%u\n", device_mac[0], device_mac[1], device_mac[2], device_mac[3], device_mac[4], device_mac[5]);
        printf("[mystrom-utils]     Device type: %u - Device flags: %02X\n", device_type, device_flags);
    } while (true);

    printf("[mystrom-utils] Receiving finished!\n");

    return true;
}

void socket_cleanup()
{
    closesocket(ReceiveSocket);
    WSACleanup();
}

// Detect

void detect_devices() {
    devices.clear();
    printf("[mystrom] Checking for MyStrom devices...\n");

    socket_receive();
}

// Main

int main() {
    socket_setup();

    detect_devices();

    socket_cleanup();
}
