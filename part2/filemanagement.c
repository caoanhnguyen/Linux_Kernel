#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void display_files() {
    struct dirent *de;
    DIR *dr = opendir(".");
    if (dr == NULL) {
        printf("Could not open current directory");
        return;
    }
    printf("List of files and directories:\n");
    while ((de = readdir(dr)) != NULL) {
        printf("%s\n", de->d_name);
    }
    closedir(dr);
}

void create_file(char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error creating file\n");
        return;
    }
    printf("File created successfully\n");
    fclose(fp);
}

void delete_file(char* filename) {
    if (remove(filename) == 0) {
        printf("File deleted successfully\n");
    } else {
        printf("Error deleting file\n");
    }
}

void display_file_content(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }
    char ch;
    printf("Contents of file:\n");
    while ((ch = fgetc(fp)) != EOF) {
        printf("%c", ch);
    }
    fclose(fp);
}

int main() {
    int choice;
    char filename[100];

    while(1) {
        printf("Choose an option:\n");
        printf("1. Display files and directories\n");
        printf("2. Create a file\n");
        printf("3. Delete a file\n");
        printf("4. Display file content\n");
        printf("5. Exit\n");

        scanf("%d", &choice);

        switch(choice) {
            case 1:
                display_files();
                break;
            case 2:
                printf("Enter file name: ");
                scanf("%s", filename);
                create_file(filename);
                break;
            case 3:
                printf("Enter file name: ");
                scanf("%s", filename);
                delete_file(filename);
                break;
            case 4:
                printf("Enter file name: ");
                scanf("%s", filename);
                display_file_content(filename);
                break;
            case 5:
                exit(0);
            default:
                printf("Invalid choice\n");
        }
    }
    return 0;
}

