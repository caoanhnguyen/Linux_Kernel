#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>        // for close()
#include <sys/socket.h>    // for socket(), bind(), listen(), accept()
#include <arpa/inet.h>     // for inet_pton()
#include <pthread.h>       // for threads
#include <stdbool.h>
#include <sys/stat.h>

// Định nghĩa loại tin nhắn
#define MSG_TYPE_CHAT 1
#define MSG_TYPE_FILE 2
#define FILE_CHUNK_SIZE 1024

// Định nghĩa cấu trúc gói tin
struct MessagePacket {
    int type;           // Loại tin nhắn (1: chat, 2: file)
    char sender[100];   // Người gửi
    char content[1024]; // Nội dung tin nhắn hoặc chunk file
    char filename[256]; // Tên file (nếu gửi file)
    int filesize;       // Kích thước file (nếu gửi file)
    int chunk_id;       // ID của chunk (nếu gửi file)
    int total_chunks;   // Tổng số chunk (nếu gửi file)
};

// Cấu trúc chứa:
// mã số socket cho kết nối client
// địa chỉ ip và cổng
// mã lỗi nếu kết nối không thành công
// cho biết kết nối thành công hay không
struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void sendReceivedMessageToTheOtherClients(struct MessagePacket* packet, int socketFD);
struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;
pthread_mutex_t criticalSection;

void* receiveAndProcessIncomingDataOnSeparateThread(void* lpParam) {
    int socketFD = (int)(intptr_t)lpParam;  // Cast back from void* to int
    
    while (1) {
        struct MessagePacket packet;
        int amountReceived = recv(socketFD, &packet, sizeof(packet), 0);
        
        if (amountReceived > 0) {
            if (packet.type == MSG_TYPE_CHAT) {
                printf("%s: %s\n", packet.sender, packet.content);
            } else if (packet.type == MSG_TYPE_FILE) {
                printf("Nhận chunk file %d/%d của file %s từ %s\n", 
                       packet.chunk_id, packet.total_chunks, packet.filename, packet.sender);
            }
            
            // Gửi tin nhắn hoặc file đến các client khác
            sendReceivedMessageToTheOtherClients(&packet, socketFD);
        }
        
        if (amountReceived == 0) {
            break;
        }
    }
    
    close(socketFD);
    pthread_exit(0);
}

void sendReceivedMessageToTheOtherClients(struct MessagePacket* packet, int socketFD) {
    pthread_mutex_lock(&criticalSection);
    for (int i = 0; i < acceptedSocketsCount; i++) {
        if (acceptedSockets[i].acceptedSocketFD != socketFD) {
            send(acceptedSockets[i].acceptedSocketFD, packet, sizeof(struct MessagePacket), 0);
        }
    }
    pthread_mutex_unlock(&criticalSection);
}

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD) {
    // Cấu trúc chứa thông tin về địa chỉ client
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(struct sockaddr_in);
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
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        return 1;
    }
    
    if (listen(serverSocketFD, 10) < 0) {
        perror("Listen failed");
        return 1;
    }
    
    // Tạo thư mục để lưu file nhận được
    mkdir("received_files", 0777);
    
    pthread_mutex_init(&criticalSection, NULL);
    printf("Server đang chạy. Đang đợi kết nối...\n");
    
    while (1) {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        
        if (clientSocket->acceptedSuccessfully) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientSocket->address.sin_addr), clientIP, INET_ADDRSTRLEN);
            printf("Client mới kết nối từ %s:%d\n", clientIP, ntohs(clientSocket->address.sin_port));
            
            acceptedSockets[acceptedSocketsCount++] = *clientSocket;
            pthread_t thread;
            pthread_create(&thread, NULL, receiveAndProcessIncomingDataOnSeparateThread, (void*)(intptr_t)clientSocket->acceptedSocketFD);
            pthread_detach(thread);  // Detach thread to run independently
        } else {
            printf("Không thể chấp nhận kết nối. Lỗi: %d\n", clientSocket->error);
            free(clientSocket);
        }
    }
    
    close(serverSocketFD);
    pthread_mutex_destroy(&criticalSection);
    return 0;
}