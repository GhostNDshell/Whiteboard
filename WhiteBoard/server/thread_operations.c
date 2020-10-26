#include "serverUtils.h"

int count_threads(Threads* threads){
        Threads* tmp = threads;

        int count=0;

        while(threads != NULL) {

                threads = threads->next;
                count++;
        }

        return count;

}

int new_thread(char *topic_name, char *thread_name, char* username){
        char path_threads[PATH_SIZE] = "../topics/";
        strcat(path_threads, topic_name);
        strcat(path_threads, "/");
        strcat(path_threads, thread_name);
        strcat(path_threads, "/");

        if (fork() == 0) {
                execl("/bin/mkdir", "mkdir", path_threads, NULL);
                exit(0);

        }
        return 1;
}

int is_thread(char* thread_name, int shm_id){

        Topics* topic_list;
        topic_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
        topic_list = topic_list->next;

        int found = 0;


        while ((topic_list != NULL) && (found == 0)) {

                Threads* thread_list;
                thread_list = topic_list->threads;

                while (thread_list != NULL) {

                        if (strcmp(thread_list->name, thread_name) == 0) {
                                found = 1;
                                break;
                        }

                        thread_list = thread_list->next;
                }

                topic_list = topic_list->next;
        }

        // Share memory Detachment
        shmdt(topic_list);

        return found;
}

void create_thread(char* topic_name,  int socket, int shm_id, char* username){

  char thread_name[BUF_SIZE];
  int conn_status;

  if(is_topic(topic_name, shm_id)) { //check if the topic already exists in the topic list


    conn_status = write(socket, "\t\t\t <[^_^]> Insert thread name!\n", BUF_SIZE);
    //if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
    conn_status = read(socket, thread_name, BUF_SIZE);
    //if(check_conn(conn_status) == 0) exit(0);

    strtok(thread_name, "\n");


    //check if the thread name has been sent
    if(is_thread(thread_name, shm_id)) { //check if the thread exists in the thread list
      write(socket, "\t\t\t (ツ)_/¯ This thread already exists!\n", BUF_SIZE);
    }
    else
    {
      new_thread(topic_name, thread_name, username); //create thread
      conn_status = write(socket, "\t\t\t <[^_^]> Thread created!\n", BUF_SIZE);
      //if(check_conn(conn_status) == 0) exit(0);
    }
  }
  else{
    conn_status = write(socket, "\t\t\t (ツ)_/¯ Topic does not exist!\n", BUF_SIZE);
    //if(check_conn(conn_status) == 0) exit(0);
  }

}

Threads* instance_thread(Threads new_thread){
        Threads *new_node = NULL;
        new_node = (Threads*) malloc(sizeof(Threads));

        *new_node = new_thread;
        new_node->next = NULL;

        return new_node;
}

Threads* append_thread(Threads* threads, Threads new_thread){
        if (threads == NULL) return instance_thread(new_thread);

        Threads* tmp = threads;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = instance_thread(new_thread);

        return threads;
}

void list_thread(int socket, int shm_id){

  Topics* topics_list;
  topics_list = (Topics*) shmat(shm_id, NULL, O_RDONLY);
  topics_list = topics_list->next;

  int conn_status;
  char topic_name[BUF_SIZE];
  char operation[BUF_SIZE];

  if (!write(socket, "\t\t\t<[^_^|> List thread from which topic ?\n", BUF_SIZE)) {
    perror("write, thread menu");
  }

  conn_status = read(socket, topic_name, BUF_SIZE);
  if(check_conn(conn_status) == 0) exit(0); //TO-DO -Will check if necessary
  strtok(topic_name, "\n");

  if(is_topic(topic_name, shm_id) == 1){

    write(socket, "start_sequence", BUF_SIZE);
    while (topics_list != NULL) {
      char tmp_topic[BUF_SIZE*2];
      sprintf(tmp_topic, "%s", topics_list->name);
      if (strcmp(tmp_topic, topic_name)==0){
        Threads* threads_list;
        threads_list = topics_list->threads;
        while (threads_list != NULL) {
          char tmp_thread[BUF_SIZE*2];
          sprintf(tmp_thread, "\t\t\t├──() %s\n", threads_list->name);
          write(socket, tmp_thread, BUF_SIZE);
          threads_list = threads_list->next;
        }
      }
      topics_list = topics_list->next;//End if 2
    }//End While
    shmdt(topics_list);
    write(socket, "end_sequence", BUF_SIZE);
  }//End if 1
  else write(socket, "\t\t\t (ツ)_/¯ Topic not found!\n", BUF_SIZE);
}
