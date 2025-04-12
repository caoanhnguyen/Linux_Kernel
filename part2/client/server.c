#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

SOCKET createTCPIpv4Socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in* createIPv4Address(char* ip, int port) {
    struct sockaddr_in* address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if (strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &(address->sin_addr));

    return address;
}

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void acceptNewConnectionAndReceiveAndPrintItsData(int serverSocketFD);
void receiveAndPrintIncomingData(int socketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

DWORD WINAPI receiveAndPrintIncomingDataOnSeparateThread(LPVOID lpParam);

void sendReceivedMessageToTheOtherClients(char* buffer, int socketFD);

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;

CRITICAL_SECTION criticalSection;

void startAcceptingIncomingConnections(int serverSocketFD) {
    while (true)
    {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;

        CreateThread(NULL, 0, receiveAndPrintIncomingDataOnSeparateThread, (LPVOID)clientSocket->acceptedSocketFD, 0, NULL);
    }
}

DWORD WINAPI receiveAndPrintIncomingDataOnSeparateThread(LPVOID lpParam) {
    int socketFD = (int)lpParam;
    char buffer[1024];

    while (true)
    {
        int amountReceived = recv(socketFD, buffer, 1024, 0);

        if (amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n", buffer);

            sendReceivedMessageToTheOtherClients(buffer, socketFD);
        }

        if (amountReceived == 0)
            break;
    }

    closesocket(socketFD);
    ExitThread(0);
}

void receiveAndPrintIncomingData(int socketFD) {
    char buffer[1024];

    while (true)
    {
        int amountReceived = recv(socketFD, buffer, 1024, 0);

        if (amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n", buffer);

            sendReceivedMessageToTheOtherClients(buffer, socketFD);
        }

        if (amountReceived == 0)
            break;
    }

    closesocket(socketFD);
}

void sendReceivedMessageToTheOtherClients(char* buffer, int socketFD) {
    EnterCriticalSection(&criticalSection);

    for (int i = 0; i < acceptedSocketsCount; i++)
    {
        if (acceptedSockets[i].acceptedSocketFD != socketFD)
        {
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        }
    }

    LeaveCriticalSection(&criticalSection);
}

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &clientAddressSize);

    struct AcceptedSocket* acceptedSocket = (struct AcceptedSocket*)malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;

    if (!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("Failed to initialize Winsock. Error code: %d\n", result);
        return 1;
    }

    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in* serverAddress = createIPv4Address("", 2000);

    int bindResult = bind(serverSocketFD, (struct sockaddr*)serverAddress, sizeof(*serverAddress));
    if (bindResult == 0)
        printf("Socket was bound successfully\n");

    int listenResult = listen(serverSocketFD, 10);

    InitializeCriticalSection(&criticalSection);

    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD, SD_BOTH);
    closesocket(serverSocketFD);

    WSACleanup();

    return 0;
}
