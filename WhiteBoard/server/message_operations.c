#include "serverUtils.h"

Messages* instance_message(Messages new_message){
        Messages *new_node = NULL;
        new_node = (Messages*) malloc(sizeof(Messages));

        *new_node = new_message;
        new_node->next = NULL;

        return new_node;
}

Messages* append_message(Messages* messages, Messages new_message){
        if (messages == NULL) return instance_message(new_message);

        Messages* tmp = messages;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = instance_message(new_message);

        return messages;
}

Messages read_message(char* path){
        Messages message;

        int fd = open(path, O_RDONLY, 0777);
        if (fd < 0) {
                perror("open");
                message.id = -1;
                return message;
        }

        char message_string[64];
        int nbytes;

        if ((nbytes = read(fd, message_string, 64)) < 0) {
                perror("read");
                message.id = -1;
                return message;
        }

        message.id = 0;
        strcpy(message.author, strtok(message_string, "\n"));
        strcpy(message.content, strtok(NULL, "\n"));


        close(fd);

        return message;
}

CMessages get_message_id(int target_id, int shm_id){
        CMessages result;

        Topics* topics_list;
        topics_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
        topics_list = topics_list->next;

        int found = 0;

        while (topics_list != NULL && found == 0) {  //check topics e move on the list

                Threads* threads_list;
                threads_list = topics_list->threads;

                while (threads_list != NULL && found == 0) { //check threads and move on the list

                        Messages* messages_list;
                        messages_list = threads_list->messages;

                        while (messages_list != NULL) {

                                if (messages_list->id == target_id) {
                                        found = 1;

                                        result.id = messages_list->id; // == target_id
                                        strcpy(result.author, messages_list->author);
                                        strcpy(result.content, messages_list->content);
                                        strcpy(result.topic_name, topics_list->name);
                                        strcpy(result.thread_name, threads_list->name);

                                        break;

                                }

                                messages_list = messages_list->next;
                        }

                        threads_list = threads_list->next;
                }

                topics_list = topics_list->next;
        }

        shmdt(topics_list);


        if (found != 1) result.id = -1;
        return result;

}

void message_status(int socket, int shm_id){
  int conn_status;
  char message_id[BUF_SIZE];
  conn_status = write(socket, "\t\t\t <[^_^]> Which message status? (message id)\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  conn_status = read(socket, message_id, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(message_id, "\n");
  int target_id = atoi(message_id);


  CMessages fnd_message = get_message_id(target_id, shm_id);

  if (fnd_message.id != -1) {
    if (fnd_message.status != 0 ) {

      conn_status = write(socket, "\t\t\t <[^_^]> Message read!\n", BUF_SIZE);// // Messsge found in the shared memory
      if(check_conn(conn_status) == 0) exit(0);
    }else {
      conn_status = write(socket, "\t\t\t <[^_^]> Message not read yet!\n", BUF_SIZE);// // Messsge found in the shared memory
      if(check_conn(conn_status) == 0) exit(0);
    }

  } else {
    write(socket, "\t\t\t (ツ)_/¯ Message not found!\n", BUF_SIZE);// Message not found
  }

}

void print_message_informations(int socket, int shm_id){

  int conn_status;

  char message_id[BUF_SIZE];
  conn_status = write(socket, "\t\t\t <[^_^]> Select a message! (message id)\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  conn_status = read(socket, message_id, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(message_id, "\n");
  int target_id = atoi(message_id);

  if (message_id != NULL || target_id > 1) {

    CMessages fnd_message = get_message_id(target_id, shm_id);

    if (fnd_message.id != -1) { //Message is found

      conn_status = write(socket, "start_sequence", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);

      char tmp[512] = "";

      sprintf(tmp, "Status: %d\n", fnd_message.status);
      conn_status = write(socket, tmp, BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);

      sprintf(tmp, "Author: %s\n", fnd_message.author);
      conn_status = write(socket, tmp, BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);

      sprintf(tmp, "Content: %s\n", fnd_message.content);
      conn_status = write(socket, tmp, 512);
      if(check_conn(conn_status) == 0) exit(0);

      sprintf(tmp, "Topic: %s\n", fnd_message.topic_name);
      conn_status = write(socket, tmp, BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);

      sprintf(tmp, "Thread: %s\n", fnd_message.thread_name);
      conn_status = write(socket, tmp, BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);


      conn_status = write(socket, "end_sequence", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);


    } else {
      write(socket, "\t\t\t (ツ)_/¯ Message not found!\n", BUF_SIZE);//Messsage not found
    }
  } else write(socket, "\t\t\t (ツ)_/¯ Invalid message\n", BUF_SIZE);

}

void reply_message(char* username, int socket, int shm_id){

  int conn_status;

  char message_id[BUF_SIZE];
  conn_status = write(socket, "\t\t\t <[^_^]> Select a message! (message id)\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  conn_status = read(socket, message_id, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(message_id, "\n");

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

  int target_id = atoi(message_id);

  if (target_id > 1) {

    CMessages fnd_message = get_message_id(target_id, shm_id);

    if (fnd_message.id != -1) { //Message found

      fnd_message.status = 1;

      char path[BUF_SIZE*3]; //May change later
      sprintf(path, "../topics/%s/%s", fnd_message.topic_name, fnd_message.thread_name);

      conn_status = write(socket, "\t\t\t <[^_^]> Enter a message!\n", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);
      // read input
      char msg_content[BUF_SIZE_LARGE];
      conn_status = read(socket, msg_content, BUF_SIZE_LARGE);
      if(check_conn(conn_status) == 0) exit(0);

      char tmp [BUF_SIZE_LARGE+10];
      sprintf(tmp, "[Reply to #%d]: ", fnd_message.id);
      strcat(tmp, msg_content);

      Messages new_message;

      // Lock semaphore (counter.txt)
      operation.sem_num = 1; // 2nd semaphore
      operation.sem_op  = -1;// Lock operation (Decrement)
      operation.sem_flg = 0; // Sem flag
      semop(sem_id, &operation, 1);
      // Sem is locked!

      new_message.id = increment_id();

      // Unlock semaphore (counter.txt)
      operation.sem_num = 1; // 2nd semaphore
      operation.sem_op  = 1;// Lock operation (Increment)
      operation.sem_flg = 0; // Sem flag
      semop(sem_id, &operation, 1);
      // Sem is unlocked!

      strcpy(new_message.author, username);
      strcpy(new_message.content, tmp);

      save_message(path, new_message);

      write(socket, "\t\t\t <[^_^]> Message sent!\n", BUF_SIZE);

    } else {
      write(socket, "\t\t\t (ツ)_/¯ Message not found!\n", BUF_SIZE);// Message Not found
    }

  } else write(socket, "\t\t\t (ツ)_/¯ Please insert a message id!\n", BUF_SIZE);
}

int save_message(char* path, Messages message) {

        char path_message[BUF_SIZE] = "";

        sprintf(path_message, "%s/%d", path, message.id);
        //printf("///%s\n",path_message );



        int fd ;
        if ((fd = open(path_message, O_CREAT | O_RDWR, 0777))< 0) {
                perror("open path");
                return 0;
        }

        char string[32];
        strcpy(string, message.author);
        strcat(string, " says:");
        strcat(string, "\n");
        strcat(string, message.content);
        int nbytes = write(fd, string, strlen(string));

        if (nbytes < 0) {
                perror("write");
                return 0;
        }


        close(fd);
        return 1;

}

void write_messgae(char* username, int socket, int shm_id){

  int conn_status;
  char thread_name[BUF_SIZE];
  char topic_name[BUF_SIZE];
  char tmp_thread[BUF_SIZE];

  conn_status = write(socket, "\t\t\t d[^_^]b Which Thread ?\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  conn_status = read(socket, thread_name, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(thread_name, "\n");

  Topics* topics_list;
  topics_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
  topics_list = topics_list->next;


  if(is_thread(thread_name, shm_id) == 1){

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

    while (topics_list != NULL) {  //check topics e move on the list

      Threads* threads_list;
      threads_list = topics_list->threads;
    //  topic_name = topics_list->name;
      sprintf(topic_name, "%s", topics_list->name);
      //tmp_thread = threads_list->name;
      while(threads_list != NULL){
        sprintf(tmp_thread, "%s", threads_list->name);


        if (strcmp(tmp_thread, thread_name ) == 0){

          conn_status = write(socket, "\t\t\t <[^_^]> Enter a message!\n", BUF_SIZE);
          if(check_conn(conn_status) == 0) exit(0);

          char msg_content[BUF_SIZE_LARGE];
          conn_status = read(socket, msg_content, BUF_SIZE_LARGE);
          if(check_conn(conn_status) == 0) exit(0);

          Messages new_message;

          // Lock semaphore (counter.txt)
          operation.sem_num = 1;   // 2nd semaphore
          operation.sem_op  = -1;  // Lock operation (Decrement)
          operation.sem_flg = 0;   // Sem flag
          semop(sem_id, &operation, 1);
          // Sem is locked!

          new_message.id = increment_id();

          // Unlock semaphore (counter.txt)
          operation.sem_num = 1;   // 2nd semaphore
          operation.sem_op  = 1;  // Lock operation (Increment)
          operation.sem_flg = 0;   // Sem flag
          semop(sem_id, &operation, 1);
          // Sem is unlocked!


          strcpy(new_message.author, username);
          strcpy(new_message.content, msg_content);

          char path[BUF_SIZE] = "../topics/";
          strcat(path, topic_name);
        //     printf("%s\n",path );
          strcat(path, "/");
        //      printf("%s\n",path );
          strcat(path, thread_name);
          //  printf("%s\n",path );

          //save message
          if (save_message(path, new_message) == 1) {
            conn_status = write(socket, "\t\t\t <[^_^]> Message saved!\n", BUF_SIZE);
            if(check_conn(conn_status) == 0) exit(0);
            break;
          } else write(socket, "\t\t\t (ツ)_/¯ Message not saved!\n", BUF_SIZE);
        }
        threads_list = threads_list->next;
      }
      topics_list = topics_list->next;
    }
    write(socket, "\t\t\t (ツ)_/¯ Thread not found!\n", BUF_SIZE);
    shmdt(topics_list);
  }
}

void list_message(int socket, int shm_id){

    Topics* topics_list;
    topics_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
    topics_list = topics_list->next;

    int conn_status;
    char thread_name[BUF_SIZE];

    if (!write(socket, "\t\t\t <[^_^]> List message from which thread?\n", BUF_SIZE)) {
      perror("write, message menu");
    }

    conn_status = read(socket, thread_name, BUF_SIZE);
    if(check_conn(conn_status) == 0) exit(0);
    strtok(thread_name, "\n");

    if(is_thread(thread_name, shm_id) == 1){

      while (topics_list != NULL) {

        Threads* threads_list;
        threads_list = topics_list->threads;

        while (threads_list != NULL)
        {
          char tmp_thread[BUF_SIZE*2];
          sprintf(tmp_thread, "%s", threads_list->name);

          if (strcmp(tmp_thread, thread_name)==0){

            write(socket, "start_sequence", BUF_SIZE);

            Messages* messages_list;
            messages_list = threads_list->messages;

            while (messages_list != NULL) {
              char tmp_message[BUF_SIZE*6];
              sprintf(tmp_message, "\t\t\t├──> (#%d) %s: %s\n", messages_list->id, messages_list->author, messages_list->content);
              write(socket, tmp_message, BUF_SIZE);
              messages_list = messages_list->next;
            }
          }
          threads_list = threads_list->next;
        }
        topics_list = topics_list->next;//End if 2
      }
      shmdt(topics_list);
      write(socket, "end_sequence", BUF_SIZE);
    }

    else write(socket, "\t\t\t (ツ)_/¯ Thread not found!\n", BUF_SIZE);

}
