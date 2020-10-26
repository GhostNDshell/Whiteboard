#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 64

int create_user(){

        printf("<[^_^]> Enter username: \n");
        char username[BUF_SIZE];
        scanf("%s", username);

        printf("<[^_^]>Enter password: \n");
        char password[BUF_SIZE];
        scanf("%s", password);

        char path[64] = "users/";
        strcat(path, username);

        int fd;
        if ((fd = open(path, O_WRONLY | O_CREAT, 0777)) < 0) {
                perror("open");
                return 0;
        }

        int nbytes;
        if ((nbytes= write(fd, password, strlen(password))) < 0) {
                perror("write");
                return 0;
        }

        close(fd);
        return 1;
}


int delete_user(){

        printf("Enter username: \n");
        char username[BUF_SIZE];
        scanf("%s", username);

        char path[BUF_SIZE] = "users/";
        strcat(path, username);


        int fd = remove(path);
        if (fd < 0) {
                perror("remove");
                return 0;
        }

        return 1;
}


int main(int argc, char const *argv[]) {

        char operation[BUF_SIZE];

        while (1) {
          printf("%s\n", "\n\t (ツ)_/¯ WELCOME ADMIN <[^_^]> \n\n\t\t\tMENU\n\n\t\t 1 - Create user\n\t\t 2 - Delete user\n\n\tPlease, Select an operation\n");


                scanf("%s", operation);

                char* command = strtok(operation, "\n");

                if (strcmp(command, "1") == 0) {
                        if (create_user() == 1) {
                                printf("<[^_^]> User created\n");
                        }else printf("<[^_^]> Error : User not created\n");
                }
                else if (strcmp(command, "2") == 0) {
                        if (delete_user() == 1) {
                                printf("<[^_^]> User deleted\n");
                        }else printf("<[^_^]> Error : User not deleted\n");

                }
                else printf("(ツ)_/¯ Invalid operation\n");

        }

        return 0;
}
