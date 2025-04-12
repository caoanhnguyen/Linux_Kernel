#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>

void list_network_interfaces();
void get_interface_status(const char *interface_name);
void enable_interface(const char *interface_name);
void disable_interface(const char *interface_name);
void change_ip_add(const char *interface_name);
int main() {
    int choice;
    char interface_name[100];

    do {
        printf("\nNetwork Interface Management Menu:\n");
        printf("1. List network interfaces\n");
        printf("2. Enable an interface\n");
        printf("3. Disable an interface\n");
	printf("4. Change ip interface\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                list_network_interfaces();
                break;
            case 2:
                printf("Enter the name of the interface to enable: ");
                scanf("%s", interface_name);
                enable_interface(interface_name);
                break;
            case 3:
                printf("Enter the name of the interface to disable: ");
                scanf("%s", interface_name);
                disable_interface(interface_name);
                break;
	    case 4:
                printf("Enter the name of the interface to change: ");
                scanf("%s", interface_name);
                change_ip_add(interface_name);
                break;
            case 5:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while(choice != 5);

    return 0;
}

void list_network_interfaces() {
    struct ifaddrs *ifaddr; // giao dien mang
    struct ifreq ifr; // ds mang 
    int family, s; // loai giao dien va ma loi
    char host[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    
    printf("Network Interface Information:\n");
    
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        
        family = ifa->ifa_addr->sa_family;
        
        printf("%-8s %s (%d)\n",
               ifa->ifa_name,
               (family == AF_PACKET) ? "AF_PACKET" :
               (family == AF_INET) ? "AF_INET" :
               (family == AF_INET6) ? "AF_INET6" : "???",
               family);
        
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST,
                            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\t\taddress: <%s>\n", host);
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            printf("\t\tStatus: ");
            strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ);
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0) {
                perror("socket");
                exit(EXIT_FAILURE);
            }
            if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
                perror("ioctl");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            if (ifr.ifr_flags & IFF_UP) {
                printf("UP\n");
            } else {
                printf("DOWN\n");
            }
            close(sockfd);
        }
    }
    
    freeifaddrs(ifaddr);
}

void enable_interface(const char *interface_name) {
    char command[100];
    sprintf(command, "sudo ifconfig %s up", interface_name);
    
    if (system(command) == -1) {
        perror("Error enabling interface");
        exit(EXIT_FAILURE);
    }

    printf("Interface %s has been enabled\n", interface_name);
}

void disable_interface(const char *interface_name) {
    char command[100];
    sprintf(command, "sudo ifconfig %s down", interface_name);

    if (system(command) == -1) {
        perror("Error disabling interface");
        exit(EXIT_FAILURE);
    }

    printf("Interface %s has been disabled\n", interface_name);
}
void change_ip_add(const char *interface_name){
    char ip_address[20];     // Địa chỉ IP mới
    char command[100];       // Lệnh để thực thi
    char commanddown[100]; 
    char dhcp[100];
    char deleteIp[100];
    char up[100];
  // Nhập địa chỉ IP mới
    printf("Enter new IP address: ");
    scanf("%s", ip_address);

    // Tạo lệnh để thay đổi địa chỉ IP
    sprintf(command, "sudo ip addr add %s dev %s", ip_address, interface_name);
    sprintf(commanddown, "sudo ifconfig %s down", interface_name);
    sprintf(dhcp, "sudo dhclient -r %s", interface_name);
    sprintf(deleteIp, "sudo ip addr flush dev %s", interface_name);
    sprintf(up, "sudo ifconfig %s up", interface_name);
    system(commanddown);
int i = 1;
int flag = 1;
while(i < 5){
   if( system(dhcp)){
flag = 1;
};
    system(deleteIp);
    system(command);
    system(up);
i++;
}
if(flag){
printf("success");   
}else{
perror("Error executing command.\n");
}
    // Thực thi lệnh

   
}
