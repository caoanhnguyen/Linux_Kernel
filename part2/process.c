#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void list_processes() {
    printf("List of processes:\n");
    system("ps -ax");
}

void signal_process() {
    pid_t pid;
    int sig;
    printf("Enter process ID: ");
    scanf("%d", &pid);
    printf("Enter signal number: ");
    scanf("%d", &sig);
    if (kill(pid, sig) == -1) {
        perror("Error sending signal");
    } else {
        printf("Signal sent to process %d\n", pid);
    }
}

int main() {
    int choice;
    while (1) {
        printf("\nProcess Management\n");
        printf("1. List processes\n");
        printf("2. Send signal to process\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                list_processes();
                break;
            case 2:
                signal_process();
                break;
            case 3:
                exit(0);
            default:
                printf("Invalid choice\n");
        }
    }
    return 0;
}
