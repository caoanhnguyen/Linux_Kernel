#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
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

// Cấu trúc toàn cục để lưu file đang nhận
typedef struct {
    char filename[256];
    char sender[100];
    FILE* file;
    int received_chunks;
    int total_chunks;
    int active;
} ReceivedFile;

ReceivedFile current_files[10];
int file_count = 0;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

// Hàm nhận tin nhắn từ server
void* recvMsg(void* sock)
{
    int their_sock = ((int)sock);
    struct MessagePacket packet;
    
    // Tạo thư mục để lưu file nhận được
    mkdir("received_files", 0777);
    
    while (recv(their_sock, &packet, sizeof(packet), 0) > 0) 
    {
        if (packet.type == MSG_TYPE_CHAT) {
            // Xử lý tin nhắn chat
            printf("%s: %s", packet.sender, packet.content);
        } 
        else if (packet.type == MSG_TYPE_FILE) {
            // Xử lý nhận file
            pthread_mutex_lock(&file_mutex);
            
            // Tìm file trong danh sách nếu đã tồn tại
            int file_index = -1;
            for (int i = 0; i < file_count; i++) {
                if (current_files[i].active && 
                    strcmp(current_files[i].filename, packet.filename) == 0 &&
                    strcmp(current_files[i].sender, packet.sender) == 0) {
                    file_index = i;
                    break;
                }
            }
            
            // Nếu chunk đầu tiên, tạo file mới
            if (packet.chunk_id == 0) {
                // Nếu file_index đã tồn tại, đóng file cũ
                if (file_index >= 0 && current_files[file_index].file) {
                    fclose(current_files[file_index].file);
                } else {
                    // Tìm slot trống hoặc thêm mới vào mảng
                    file_index = -1;
                    for (int i = 0; i < file_count; i++) {
                        if (!current_files[i].active) {
                            file_index = i;
                            break;
                        }
                    }
                    
                    if (file_index == -1) {
                        file_index = file_count++;
                    }
                }
                
                // Khởi tạo thông tin file mới
                char filepath[512];
                sprintf(filepath, "received_files/%s", packet.filename);
                current_files[file_index].file = fopen(filepath, "wb");
                strcpy(current_files[file_index].filename, packet.filename);
                strcpy(current_files[file_index].sender, packet.sender);
                current_files[file_index].received_chunks = 0;
                current_files[file_index].total_chunks = packet.total_chunks;
                current_files[file_index].active = 1;
                
                printf("\nĐang nhận file '%s' từ %s (%d bytes)...\n", 
                       packet.filename, packet.sender, packet.filesize);
            }
            
            // Ghi chunk vào file
            if (file_index >= 0 && current_files[file_index].file) {
                fwrite(packet.content, 1, strlen(packet.content), current_files[file_index].file);
                current_files[file_index].received_chunks++;
                
                // Nếu đã nhận đủ chunks, đóng file
                if (current_files[file_index].received_chunks >= current_files[file_index].total_chunks) {
                    fclose(current_files[file_index].file);
                    current_files[file_index].file = NULL;
                    current_files[file_index].active = 0;
                    
                    printf("\nĐã nhận file hoàn tất: %s từ %s\n", 
                           current_files[file_index].filename, current_files[file_index].sender);
                }
            }
            
            pthread_mutex_unlock(&file_mutex);
        }
    }
    
    return NULL;
}

// Hàm gửi file
void sendFile(int sock, char* username, char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        printf("Không thể mở file '%s'\n", filepath);
        return;
    }
    
    // Lấy kích thước file
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Tính toán số chunk
    int total_chunks = (filesize + FILE_CHUNK_SIZE - 1) / FILE_CHUNK_SIZE;
    
    // Lấy tên file (không có đường dẫn)
    char* filename = strrchr(filepath, '/');
    if (filename) {
        filename++;  // Bỏ qua dấu '/'
    } else {
        filename = filepath;
    }
    
    printf("Đang gửi file '%s' (%ld bytes, %d chunks)\n", filename, filesize, total_chunks);
    
    // Gửi file theo từng chunk
    char buffer[FILE_CHUNK_SIZE];
    struct MessagePacket packet;
    int chunk_id = 0;
    
    while (!feof(file)) {
        // Đọc một chunk từ file
        memset(buffer, 0, FILE_CHUNK_SIZE);
        size_t bytes_read = fread(buffer, 1, FILE_CHUNK_SIZE, file);
        
        if (bytes_read <= 0) break;
        
        // Chuẩn bị gói tin
        packet.type = MSG_TYPE_FILE;
        strcpy(packet.sender, username);
        strcpy(packet.content, buffer);
        strcpy(packet.filename, filename);
        packet.filesize = filesize;
        packet.chunk_id = chunk_id++;
        packet.total_chunks = total_chunks;
        
        // Gửi gói tin
        send(sock, &packet, sizeof(packet), 0);
        
        // Chờ một chút để tránh tắc nghẽn
        usleep(10000);  // 10ms
    }
    
    fclose(file);
    printf("Đã gửi file thành công!\n");
}

void showMenu() {
    printf("\n==== MENU ====\n");
    printf("1. Chat bình thường\n");
    printf("2. Gửi file\n");
    printf("0. Thoát\n");
    printf("Lựa chọn của bạn: ");
}

int main(int argc, char* argv[])
{
    struct sockaddr_in their_addr;
    pthread_t sendt, recvt;
    int my_sock;
    int portno;
    char username[100];
    char ip[INET_ADDRSTRLEN];
    
    if (argc > 3) 
    {
        printf("Quá nhiều tham số...\n");
        exit(1);
    }
    
    portno = atoi(argv[2]);
    strcpy(username, argv[1]);
    
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
    {
        perror("Mở kênh không thành công...");
        exit(1);
    }
    
    memset(their_addr.sin_zero, '\0', sizeof(their_addr.sin_zero));
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(portno);
    their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Sửa IP theo server của bạn
    
    if (connect(my_sock, (struct sockaddr*) &their_addr, sizeof(their_addr)) < 0)
    {
        perror("Kết nối không thành công...");
        exit(1);
    }
    
    inet_ntop(AF_INET, (struct sockaddr*) &their_addr, ip, INET_ADDRSTRLEN);
    printf("Đã kết nối tới %s, bắt đầu trò chuyện\n", ip);
    
    // Tạo thread để nhận tin nhắn
    pthread_create(&recvt, NULL, recvMsg, &my_sock);
    
    int choice = 1;  // Mặc định là chế độ chat
    char input[1024];
    struct MessagePacket packet;
    
    while (1) {
        showMenu();
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);
        
        if (choice == 0) {
            break;  // Thoát
        }
        else if (choice == 1) {
            // Chế độ chat
            printf("Chế độ chat. Nhập tin nhắn (gõ 'menu' để quay lại menu):\n");
            
            while (1) {
                memset(input, 0, sizeof(input));
                fgets(input, sizeof(input), stdin);
                
                // Kiểm tra nếu người dùng muốn quay lại menu
                if (strcmp(input, "menu\n") == 0) {
                    break;
                }
                
                // Chuẩn bị gói tin chat
                packet.type = MSG_TYPE_CHAT;
                strcpy(packet.sender, username);
                strcpy(packet.content, input);
                
                // Gửi tin nhắn
                int len = send(my_sock, &packet, sizeof(packet), 0);
                if (len < 0) {
                    perror("Gửi tin nhắn không thành công...");
                    exit(1);
                }
            }
        }
        else if (choice == 2) {
            // Chế độ gửi file
            printf("Chế độ gửi file. Nhập đường dẫn đầy đủ đến file (hoặc 'menu' để quay lại):\n");
            
            memset(input, 0, sizeof(input));
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;  // Loại bỏ ký tự newline
            
            // Kiểm tra nếu người dùng muốn quay lại menu
            if (strcmp(input, "menu") == 0) {
                continue;
            }
            
            // Gửi file
            sendFile(my_sock, username, input);
        }
    }
    
    pthread_cancel(recvt);
    close(my_sock);
    return 0;
}