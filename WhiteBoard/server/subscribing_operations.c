#include "serverUtils.h"

void subscribe(char* username, int socket, int shm_id) {

  int conn_status;
  char topic_name[BUF_SIZE];

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


  conn_status = write(socket, "\t\t\t <[^_^]> Which topic would you like to sub to?\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  conn_status = read(socket, topic_name, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(topic_name, "\n");

  if (is_topic(topic_name, shm_id)) { // Topic found

    // Lock semaphore (Topics handling)
    operation.sem_num = 0; // 1st semaphore
    operation.sem_op  = -1;// Lock operation (Decrement)
    operation.sem_flg = 0; // Sem flag
    semop(sem_id, &operation, 1);
    // Sem is locked!

    int subscription = add_subscriber(topic_name, username);
    if(subscription == 1) {
      conn_status = write(socket, "\t\t\t <[^_^]> Subscribed!\n", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);
    } else if(subscription == 2) {
      conn_status = write(socket, "\t\t\t <[^_^]> You're already subscribed!\n", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);
    } else write(socket, "\t\t\t (ツ)_/¯ Not subscribed!\n", BUF_SIZE);

    // Lock semaphore (Topics handling)
    operation.sem_num = 0;   // 1st semaphore
    operation.sem_op  = 1;  // Unlock operation (Increment)
    operation.sem_flg = 0;   // Sem flag
    semop(sem_id, &operation, 1);
    // Sem is unlocked!
  } else write(socket, "\t\t\t (ツ)_/¯ Topic not found", BUF_SIZE);
}

Subscribers* instance_subscriber(char* subscriber){
        Subscribers *new_node = NULL;
        new_node = (Subscribers*) malloc(sizeof(Subscribers));

        strcpy(new_node->name, subscriber);
        new_node->next = NULL;

        return new_node;
}

Subscribers* append_subscribers(Subscribers* subscribers, char* username){
        if (subscribers == NULL) return instance_subscriber(username);

        Subscribers* tmp = subscribers;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = instance_subscriber(username);

        return subscribers;
}

Subscribers* print_subscribers(char* path){
        Subscribers* subs_list = NULL;
        char path_subs[64];
        strcpy(path_subs, path); // topics/topicsX/
        strcat(path_subs, "subscribers.txt"); // topics/topicsX/subscribers.txt

        int fd = open(path_subs, O_RDONLY);
        if (fd < 0) {
                return NULL;
        }

        char subs[128];
        int nbytes = read(fd, subs, 128);
        if (nbytes < 1) {
                return NULL;
        }

        char* token = strtok(subs, "\n"); // token = 1st name

        while (token) {
                subs_list = append_subscribers(subs_list, token);
                token = strtok(NULL, "\n");
        }


        close(fd);
        return subs_list;
}

int is_subscribed(int fd, char* username){

        char buffer[BUF_SIZE_LARGE];
        if(read(fd, buffer, BUF_SIZE_LARGE) < 1) return 0;

        char* token = strtok(buffer, "\n");
        while (token != NULL) {
                if (strcmp(token, username) == 0) return 1;
                token = strtok(NULL, "\n");
        }

        return 0;
}

int add_subscriber(char* topic_name, char* username){
        Subscribers* subs_list;
        char path_subs[64] = "../topics/";
        strcat(path_subs, topic_name);
        strcat(path_subs, "/subscribers.txt");


        int fd;

        if ( (fd = open(path_subs, O_RDWR | O_CREAT | O_APPEND, 0777)) < 0) {
                perror("open subs");
                return 0;
        }


        if(is_subscribed(fd, username) == 1) return 2;

        int nbytes = write(fd, username, strlen(username));
        write(fd, "\n", 1);

        if (nbytes < 1) {
                perror("write");
                return 0;
        }

        close(fd);

        return 1;
}
