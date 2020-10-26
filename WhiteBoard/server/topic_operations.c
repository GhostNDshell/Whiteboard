#include "serverUtils.h"

int new_topic(char *topic_name, char* username){
  char path[PATH_SIZE] = "../topics/";
  strcat(path, topic_name);
  printf("%s\n",path );

  if(fork() == 0) {
    execl("/bin/mkdir", "mkdir", path, NULL);
    exit(0);
  }else{
    wait(NULL);
    save_owner(path, username);
  }

  return 1;
}

void create_topic(char* topic_name, char* username, int socket, int shm_id){

  int conn_status;
  if(topic_name != NULL) { //check if the topic name has been sent

      if(is_topic(topic_name, shm_id)) { //check if the topic already exists in the topic list
                write(socket, "\t\t\t (ツ)_/¯ This topic already exists!\n", BUF_SIZE);
              }//End if
      else{
              new_topic(topic_name, username); //create topic
              conn_status = write(socket, "\t\t\t <[^_^]> Topic created!\n", BUF_SIZE);
              if(check_conn(conn_status) == 0) exit(0);
            }
          }
        }

int remove_topic(char* topic_name, char* username){

          char path[PATH_SIZE] = "../topics/";
          strcat(path, topic_name);
          strcat(path, "/");

          if (strcmp(get_owner(path), username) == 0) {
                  if(fork() == 0) {
                          execl("/bin/rm", "rm", "-r", path, NULL);
                          exit(0);
                  }else {
                        wait(NULL);
                      }
                return 1;
          } else return 0;
  }

void delete_topic(char* username, int socket, int shm_id){

  int conn_status;
  char topic_name[BUF_SIZE];
  conn_status = write(socket, "\t\t\t <[^_^]> Select a Topic to delete!\n", BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  conn_status = read(socket, topic_name, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0);
  strtok(topic_name, "\n");

  if (is_topic(topic_name, shm_id) == 0) {
    conn_status = write(socket, "\t\t\t (ツ)_/¯ Topic not found!\n", BUF_SIZE);
    if(check_conn(conn_status) == 0) exit(0);
  }else{
    if (remove_topic(topic_name, username) == 1) {
      conn_status = write(socket, "\t\t\t <[^_^]> Topic deleted!\n", BUF_SIZE);
      if(check_conn(conn_status) == 0) exit(0);
    }else write(socket, "\t\t\t (ツ)_/¯ You cannot delete this topic!\n", BUF_SIZE);
  }

}

int read_topics (int shm_id, char *initial_path){

  Topics* topic_list = NULL;
  topic_list = (Topics*) shmat(shm_id, NULL, 0);
  topic_list->next = NULL;

  DIR *topics_dir = opendir(initial_path);
  if (topics_dir == NULL) {
    perror("opendir");
    return 0;
  }

  struct dirent *d0;

  // Scan the directory topics/
  while ((d0  = readdir(topics_dir)) != NULL) {
    if (d0->d_type == DT_DIR) {
      if (strcmp(d0->d_name, ".") == 0) continue;
      if (strcmp(d0->d_name, "..") == 0) continue;

      Topics tmp_topic_node;

      strcpy(tmp_topic_node.name, d0->d_name);

      // building the path of the single topic
      char path[PATH_SIZE];
      strcpy(path, initial_path);   //     topics/
      strcat(path, d0->d_name);    //     topics/topicX
      strcat(path, "/");          //     topics/topicX/

      strcpy(tmp_topic_node.owner, get_owner(path));

      Threads* tmp_threads_list = NULL;

      struct dirent *d1;

      DIR *thread_dir = opendir(path);
      if (thread_dir == NULL) {
        perror("opendir");
        return 0;
      }

      // scan the directory topics/threadX
      while ((d1 = readdir(thread_dir)) != NULL) {
        Threads tmp_thread = {"", NULL, NULL};
        if (d1->d_type == DT_DIR) {
          if (strcmp(d1->d_name, ".") == 0) continue;
          if (strcmp(d1->d_name, "..") == 0) continue;


          strcat(tmp_thread.name, d1->d_name);

          // building the path of the single thread
          char path_threads[PATH_SIZE];
          strcpy(path_threads, path);           // topics/topicX/
          strcat(path_threads, d1->d_name);     // topics/topicX/threadY
          strcat(path_threads, "/");            // topics/topicX/threadY/

          Messages* tmp_messages_list = NULL;
          struct dirent *d2;

          DIR *message_dir = opendir(path_threads);
          if (message_dir == NULL) {
            perror("opendir");
            return 0;
          }

          while ((d2 = readdir(message_dir)) != NULL) {
            if (d2->d_type == DT_REG) {
              Messages tmp_message;

              char path_messages[PATH_SIZE];
              strcpy(path_messages, path_threads);         // topics/topicX/threadY/
              strcat(path_messages, d2->d_name);         // topics/topicX/threadY/message id
              tmp_message = read_message(path_messages);
              tmp_message.id = atoi(d2->d_name);



              tmp_messages_list = append_message(tmp_messages_list, tmp_message);   //append of the single messahe to the message list, which is a field of the thread

            }
            tmp_thread.messages = tmp_messages_list;
          }
          closedir(message_dir);

          tmp_threads_list = append_thread(tmp_threads_list, tmp_thread);

        }
      }
      closedir(thread_dir);

      tmp_topic_node.threads = tmp_threads_list;
      tmp_topic_node.subscribers = print_subscribers(path);

      topic_list = append_topics(topic_list, tmp_topic_node);
    }
  }

  closedir(topics_dir);
  shmdt(topic_list);

  return 1;
}

int count_topics(Topics* topics){
        Topics* tmp = topics;

        int count = 0;
        while(tmp != NULL) {
                tmp = tmp->next;
                count++;
        }

        return count;

}

int is_topic(char* topic_name, int shm_id){

        Topics* topic_list;
        topic_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
        topic_list = topic_list->next;

        int found = 0;

        while (topic_list != NULL) {
                if (strcmp(topic_list->name, topic_name) == 0) {
                        found = 1;
                        break;
                }

                topic_list = topic_list->next;
        }


        // Share memory Detachment
        shmdt(topic_list);

        return found;

}

Topics* istance_topic(Topics new_topic){
        Topics* new_node = NULL;
        new_node = (Topics*) malloc(sizeof(Topics));

        *new_node = new_topic;
        new_node->next = NULL;

        return new_node;
}

Topics* append_topics(Topics* topics, Topics new_topic){
        if (topics == NULL) return istance_topic(new_topic);

        Topics* tmp = topics; //copying the head list value in the temporary variable
        while (tmp->next != NULL) tmp = tmp->next; //moving on to the next node
        tmp->next = istance_topic(new_topic); //append of the new node to the last node of the list

        return topics;
}

void list_topic(int socket, int shm_id){
    //print_topics(shm_id, socket, 3);
    //if(parameter == NULL) write(socket, "Missing parameter", BUF_SIZE);
    Topics* topics_list;
    topics_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
    topics_list = topics_list->next; // Workaround in order to skip the first empty node

    write(socket, "start_sequence", BUF_SIZE);

    while (topics_list != NULL) {
      char tmp_topic[BUF_SIZE*2];

      sprintf(tmp_topic, "\t\t\t├──[] %s\n", topics_list->name);

      write(socket, tmp_topic, BUF_SIZE);
      topics_list = topics_list->next;
    }
    shmdt(topics_list);

    write(socket, "end_sequence", BUF_SIZE);
//
}

char* get_owner(char* path){

  int fd;
  char path_owner[64];
  strcpy(path_owner, path);
  strcat(path_owner, "owner.txt");

  if ((fd = open(path_owner, O_RDONLY, 0777)) < 0) {
    perror("open owner get");
    return 0;
  }


  char* owner = malloc(32);

  int nbytes;
  if ((nbytes = read(fd, owner, 32)) < 0) {
    perror("read");
    return 0;
  }

  owner[strlen(owner)] = 0;
  strtok(owner, "\n");


  close(fd);

  return owner;
}

int save_owner(char* path, char* owner){
  char path_owner[64];
  strcpy(path_owner, path);
  strcat(path_owner, "/owner.txt");


  int fd;
  if ((fd = open(path_owner, O_CREAT | O_RDWR, 0777)) < 0) {
    perror("open owner save");
    return 0;
  }

  int nbytes;
  if ( (nbytes= write(fd, owner, strlen(owner)))  < 0) {
    perror("write");
    return 0;
  }
  close(fd);
  return 1;
}
