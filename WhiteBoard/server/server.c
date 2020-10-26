#include "serverUtils.h"


int main(int argc, char const *argv[])
{

        shm_clear();

        int conn_status;

        // ---------------------- SOCKET
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int sfd = socket (AF_INET, SOCK_STREAM, 0);

        int options = 1;
        int result;
        result = setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options));
        perror("setsockopt");
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        bind(sfd, (struct sockaddr *)&address, addrlen);
        perror("bind");
        listen(sfd, 5);
        perror("listen");
        // SHARED MEMORY
        int shm_id;

        shm_id = shmget(KEY_SHM, sizeof(Topics*), IPC_CREAT | 0777);
        perror("shmget");
        printf("%s\n", "--------------SERVER ON--------------");


        if(read_topics(shm_id, "../topics/") == 0) {
                printf("error!\n");
                exit(0);
        }

        // ---------------------- SEMAPHORES
        // 1st (0) semaphore: topics/ folder
        // 2nd (1) semaphore: counter.txt

        int sem_id;
        key_t key = ftok("./server.c", 'J');
        sem_id = semget(key, 2, 0777 | IPC_CREAT);

        // sem 1: ?
        // sem 2: ?
        semctl(sem_id, 0, SETVAL, 1);
        semctl(sem_id, 1, SETVAL, 1);
        // sem 1: 1
        // sem 2: 1

        struct sembuf operation;

        char is_not_authenticated[] = "\t\t\t (ツ)_/¯ You need to be authenticated!\n";

        while (1) {
          // Listening for connections cycle
          int socket = accept(sfd, (struct sockaddr *)&address, (socklen_t*)&addrlen); // accept connections

          if (fork() == 0) { //Delegate to child process

            printf("d[o_0]b Client connected!\n");
            char command[BUF_SIZE];
            int authenticationStatus = 0;
            int registrationStatus = 0; //Registration status
            char username[BUF_SIZE];
            char password[BUF_SIZE];
            //check_conn(conn_status);


            while (1) {
              read_topics(shm_id, "../topics/");
              conn_status = read(socket, command, BUF_SIZE);
              strtok(command, "\n");


              //REGISTRATION
              if (strcmp(command, "1") == 0) {

                //Check registration status
                if (registrationStatus == 1) {
                  write(socket, "\t\t\t (ツ)_/¯ User already registered to the WHITEBOARD!\n",BUF_SIZE );
                  continue;
                }//End if

                char username[BUF_SIZE];
                char password[BUF_SIZE];

                get_input(username, password, socket);
                registrationStatus = registration(username, password, socket);
              }//End REGISTRATION

              //AUTHENTICATION
              else if (strcmp(command, "2") == 0){

                if (authenticationStatus == 2) {
                  registrationStatus = 1; //to avoid already authenticated user to use the register command
                  write(socket, "\t\t\t (ツ)_/¯ User already authenticated to the WHITEBOARD!\n",BUF_SIZE );
                  continue; //one client can't authenticate more than one user per session
                }


                get_input(username, password, socket);
                authenticationStatus = authentication(username, password, socket);



              }//END AUTHENTICATION

              //CREATE TOPIC
              else if (strcmp(command, "3") == 0) {


                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

                char topic_name[BUF_SIZE];

                conn_status = write(socket, "\t\t\t <[^_^]> Insert a topic name!\n", BUF_SIZE);
                if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
                conn_status = read(socket, topic_name, BUF_SIZE);
                if(check_conn(conn_status) == 0) exit(0);

                strtok(topic_name, "\n");

                // Lock semaphore (Topics handling)
                operation.sem_num = 0;   // 1st semaphore
                operation.sem_op  = -1;  // Lock operation (Decrement)
                operation.sem_flg = 0;   // Sem flag
                semop(sem_id, &operation, 1);
                // Sem is locked!
                create_topic(topic_name, username, socket, shm_id);
                // Lock semaphore (Topics handling)
                operation.sem_num = 0;   // 1st semaphore
                operation.sem_op  = 1;  // Unlock operation (Increment)
                operation.sem_flg = 0;   // Sem flag
                semop(sem_id, &operation, 1);
                // Sem is unlocked!

              }//END CREATE TOPIC

              //CREATE THREAD
              else if (strcmp(command, "4") == 0) {

                char topic_name[BUF_SIZE];

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE_LARGE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

                conn_status = write(socket, "\t\t\t <[^_^]>  In which Topic?\n", BUF_SIZE);
                if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
                conn_status = read(socket, topic_name, BUF_SIZE_LARGE);
                if(check_conn(conn_status) == 0) exit(0);
                strtok(topic_name, "\n");

                // Lock semaphore (Topics handling)
                operation.sem_num = 0;   // 1st semaphore
                operation.sem_op  = 1;  // Unlock operation (Increment)
                operation.sem_flg = 0;   // Sem flag
                semop(sem_id, &operation, 1);
                // Sem is unlocked!

                create_thread(topic_name, socket, shm_id, username);
                // Lock semaphore (Topics handling)
                operation.sem_num = 0;   // 1st semaphore
                operation.sem_op  = 1;  // Unlock operation (Increment)
                operation.sem_flg = 0;   // Sem flag
                semop(sem_id, &operation, 1);
                // Sem is unlocked!

              }//END CREATE THREAD

              //DELETE TOPIC
              else if (strcmp(command, "5") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }
                delete_topic(username, socket, shm_id);


              }//END DELETE TOPIC

              //LIST TOPIC - THREAD - MESSAGES
              else if (strcmp(command, "6") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }
                list(socket,shm_id);

              }//END LIST

              //APPEND NEW MESSAGE TO NEW TOPIC
              else if (strcmp(command, "7") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

                write_messgae(username, socket, shm_id);

              }

              //REPLY TO A MESSAGE IN A THREAD
              else if (strcmp(command, "8") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

                reply_message(username, socket, shm_id);

              }

              //SELECT A GIVEN MESSAGE
              else if (strcmp(command, "9") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }
                print_message_informations(socket, shm_id);


              }//END SELECT A GIVEN MESSAGE

              //MESSAGE STATUS
              else if (strcmp(command, "10") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

              message_status(socket, shm_id);

              }//END MESSAGE STATUS

              //SUBSCRIBE TOPIC
              else if (strcmp(command, "11") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

              subscribe(username, socket, shm_id);

              }//END SUBSCRIBE TOPIC

              //LOG OUT
              else if (strcmp(command, "12") == 0) {
                /* code */
                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }
                else {
                  registrationStatus = 0;
                  authenticationStatus = 0;
                  write(socket, "\t\t\t (✖╭╮✖) Logged out!\n", BUF_SIZE);
                }

              }//END LOGOUT
              
              /**else if (strcmp(command, "0") == 0){
                  char menu[] = "\tWELCOME TO WHITEBOARD\n\n\tMENU\n1 - Registration\n2 - Authentication\n3 - Create topic\n4 - Create thread\n5 - Delete topic\n6 - List (Topic, Thread, message)\n7 - New message\n8 - Reply message\n9 - Print message\n10 - Message status\n11 - Subscribe to topic\n12 - Logout\n\n\tPlease, Select an operation";

                  if (authenticationStatus == 0) {
                    conn_status = write(socket, menu, strlen(menu));
                    if(check_conn(conn_status) == 0) exit(0);
                    continue;
                  }
                }**/
              else write(socket, "\t\t\t (ツ)_/¯ Command not found!\n", BUF_SIZE);

            }//END COMMAND MANAGEMENT
          }//END CHILD DELEGATION
        }//END FOREVER WHILE

        return 0;

}//END MAIN

/**
              //COUNT
              else if (strcmp(command, "12") == 0) {

                if (authenticationStatus == 0) {
                  conn_status = write(socket, is_not_authenticated, BUF_SIZE);
                  if(check_conn(conn_status) == 0) exit(0);
                  continue;
                }

                #include "commands/count.c"

              }//END COUNT
**/ //May going to add a count function
