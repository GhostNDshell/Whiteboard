#include "clientUtils.h"

int main(int argc, char const *argv[]) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int options = 1;


        int sock = socket (AF_INET, SOCK_STREAM, 0);


        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);


        int status = connect(sock, (struct sockaddr *)&address, addrlen); //will chan
        if (status < 0) {
                printf("Failed to connect\n");
                exit(0);
        }
        else
        {

          printf("%s\n", "\n\t (ツ)_/¯ WELCOME TO WHITEBOARD <[^_^]> \n\n\t\t\tMENU\n\n\t\t 1 - Registration\n\t\t 2 - Authentication\n\t\t 3 - Create topic\n\t\t 4 - Create thread\n\t\t 5 - Delete topic\n\t\t 6 - List (Topic, Thread, message)\n\t\t 7 - New message\n\t\t 8 - Reply message\n\t\t 9 - Print message\n\t\t10 - Message status\n\t\t11 - Subscribe to topic\n\t\t12 - Logout\n\n\tPlease, Select an operation\n\n");

        }

        char command[BUF_SIZE];

        while (1)
        {
          printf("%s","(^_^) >> " );
          fgets(command, BUF_SIZE, stdin); // Take the client operation in input.
          write(sock, command, BUF_SIZE); // Sends it to the server.


                if(read(sock, command, BUF_SIZE) == 0) {
                        printf("Server unavailable\n");
                        exit(0);
                };


                if(strcmp(command, "start_sequence") == 0) {
                  while (1) {
                    read(sock, command, BUF_SIZE);
                    if (strcmp(command, "end_sequence") == 0) break;
                    else printf("%s", command);
                  }
                }
                else printf("%s\n", command);
              }
        return 0;
}
