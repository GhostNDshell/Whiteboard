#include "serverUtils.h"

void shm_clear(){
        int shmID_topics = shmget(KEY_SHM, sizeof(Topics*), IPC_CREAT | 0777);

        shmctl(shmID_topics, IPC_RMID, 0);

        return;
}

void get_input(char* username, char* password, int socket){

  int connectionStatus;
  connectionStatus = write(socket, "\t\t\t <[^_^]> Username!\n", BUF_SIZE);
  if(check_conn(connectionStatus) == 0) exit(0); //TO-DO -Will check if necessary
  connectionStatus = read(socket, username, BUF_SIZE);
  if(check_conn(connectionStatus) == 0) exit(0);
  connectionStatus = write(socket, "\t\t\t <[^_^]> Password!\n", BUF_SIZE);
  if(check_conn(connectionStatus) == 0) exit(0);
  connectionStatus = read(socket, password, BUF_SIZE);
  if(check_conn(connectionStatus) == 0) exit(0);

  strtok(username, "\n");
  strtok(password, "\n");

}

int check_conn(int conn_status) {
        if (conn_status == 0) {
                printf("(✖╭╮✖) Client Disconnected!\n");
                return 0;
        }
        return 1;
}

void list(int socket, int shm_id){

  char listMenu[] = "\n\tList menu:\n\t1 - Topics\n\t2 - Threads\n\t3 - Messages\n\t4 - All\n";

  //Print the list sub-menu operations
  if (!write(socket, listMenu, BUF_SIZE)) {
    perror("write List menu");
  }

  char parameter[BUF_SIZE];

  int conn_status;
  conn_status = read(socket, parameter, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(parameter, "\n");
  //puts(parameter);
  //int opid = atoi(parameter);

  if (strcmp(parameter, "1") == 0) {
    list_topic(socket, shm_id);
  }
  else if (strcmp(parameter, "2") == 0){
    list_thread(socket, shm_id);
  }
  else if (strcmp(parameter, "3") == 0) {
    list_message(socket, shm_id);
  }
  else if(strcmp(parameter, "4") == 0){
    map_whiteboard(shm_id, socket);
  }
  else write(socket, "\t\t\t (ツ)_/¯ Command not found!\n", BUF_SIZE);
}

int get_id(){
        // opening file
        int fd = open("../counter.txt", O_CREAT | O_RDWR, 777);
        if (fd < 0) {
                perror("open counter");
                return -1; //message id non existent
        }

        // Reading and/or writing
        char* counter = malloc(sizeof(int));
        int nbytes = read(fd, counter, sizeof(int));
        if (nbytes < 0) {
                perror("read");
                return -1;
        }
        // closing file
        close(fd);
        return atoi(counter);
}

int increment_id(){


        int new_id = get_id() + 1;
        int fd = open("../counter.txt", O_RDWR | O_TRUNC, 777);


        char* counter = malloc(sizeof(int));
        sprintf(counter, "%d", new_id); //saving in counter a string that contains the variable new_id which is an integer
        int nbytes = write(fd, counter, strlen(counter));

        if (nbytes < 0) {
                perror("write");
                return -1;
        }

        return new_id;
}

int topic_thread_exists(char* topicName, char* threadName, int shmId){

        Topics* topicList;
        topicList = (Topics*) shmat(shmId, NULL, O_RDONLY);
        topicList = topicList->next;

        int found = 0;


        while (topicList != NULL) {

                if (strcmp(topicList->name, topicName) == 0) {

                        Threads* threadList;
                        threadList = topicList->threads;
                        while (threadList != NULL) {

                                if (strcmp(threadList->name, threadName) == 0) {
                                        found = 1;
                                        break;

                                }

                                threadList = threadList->next;

                        }
                        break;
                }

                topicList = topicList->next;
        }


        shmdt(topicList);

        return found;

}

void map_whiteboard(int shmId, int socket){

        char *result = malloc(sizeof(char));
        explicit_bzero(result, sizeof(char));

        Topics* topicList;
        topicList = (Topics*) shmat(shmId, NULL, O_RDONLY);
        topicList = topicList->next; // Workaround in order to skip the first empty node


        write(socket, "start_sequence", BUF_SIZE);

        while (topicList != NULL) {
                char tmp_topic[BUF_SIZE*2];

                sprintf(tmp_topic, "\t\t\t├──> %s\n", topicList->name);

                write(socket, tmp_topic, BUF_SIZE);


                Threads* threadList;
                threadList = topicList->threads;


                while (threadList != NULL) {
                        char tmp_thread[BUF_SIZE*2];
                        sprintf(tmp_thread, "\t\t\t│\t├──> %s\n", threadList->name);


                        write(socket, tmp_thread, BUF_SIZE);

                        Messages* messages_list;
                        messages_list = threadList->messages;


                        while (messages_list != NULL) {
                                char tmp_message[BUF_SIZE*6];

                                sprintf(tmp_message, "\t\t\t│\t│\t├──> (#%d) %s: %s\n", messages_list->id, messages_list->author, messages_list->content);

                                write(socket, tmp_message, BUF_SIZE);

                                messages_list = messages_list->next;

                        }

                        threadList = threadList->next;
                }

                topicList = topicList->next;
        }

        shmdt(topicList);

        write(socket, "end_sequence", BUF_SIZE);
        return;
}

void menu(int socket){
  char menu[] = "\tWELCOME TO WHITEBOARD\n\n\n\tMENU\n1 - Registration\n2 - Authentication\n3 - Create topic\n4 - Create thread\n5 - Delete topic\n6 - List (Topic, Thread, message)\n7 - New message\n8 - Reply message\n9 - Print message\n10 - Message status\n11 - Subscribe to topic\n12 - Logout\n\n\tPlease, Select an operation";
  if (write(socket, menu, strlen(menu))) {
    perror("write menu");
  }
}
