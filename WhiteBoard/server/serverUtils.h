#ifndef utilsServer_h
#define utilServer_h
//#define RCVBUFSIZE 64
#define BUF_SIZE 64
#define BUF_SIZE_LARGE 512
#define PATH_SIZE 512
#define PORT 8080
#define KEY_SHM 1300
#define KEY_SEM 2700

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <ctype.h>
#include <strings.h>

typedef struct user {
  int id;
  char* username;
  struct user* next;

} s_utente;

typedef struct struct_messages_complete {
        int id;
        int status;
        char author[64];
        char content[254];
        char topic_name[64];
        char thread_name[64];
} CMessages;

typedef struct struct_messages {
        int id;
        char author[64];
        char content[254];
        struct struct_messages* next;
} Messages;


typedef struct struct_threads {
        char name[64];
        struct struct_messages *messages;
        struct struct_threads *next;

} Threads;


typedef struct struct_subscribers {
        char name[64];
        struct struct_subscribers *next;
} Subscribers;


typedef struct struct_topics {
        char name[64];
        char owner[64];
        struct struct_topics *next;
        struct struct_threads *threads;
        struct struct_subscribers *subscribers;
} Topics;



//REGISTRATION OPERATIONS
int check_username_available(char* username, char* password, int socket);
int registration(char* username, char* password, int socket);
int check_validity(char* username, char* password, int socket);

//AUTHENTICATION OPERATIONS
void is_authenticated(int authenticationStatus, int socket);
int authentication(char* username, char* password, int socket);
int check_credentials(char* username, char* password);

//TOPIC OPERATIONS
void list_topic(int socket, int shm_id);
int read_topics (int shm_id, char *initial_path);
void create_topic(char* topic_name, char* username, int socket, int shmId);
int is_topic(char* topicName, int shmId);
int new_topic(char *topicName, char* username);
void delete_topic(char* username, int socket, int shm_id);
int remove_topic(char* topic_name, char* username);
Topics* append_topics(Topics* topics, Topics new_topic);
char* get_owner(char* path);
int save_owner(char* path, char* owner);

//THREAD OPERATIONS
void list_thread(int socket, int shm_id);
void create_thread(char* topic_name,  int socket, int shm_id, char* username);
Threads* append_thread(Threads* threads, Threads new_thread);
int is_thread(char* thread_name, int shm_id);
int new_thread(char *topicName, char *threadName, char* username);

//MESSAGE OPERATIONS
void list_message(int socket, int shm_id);
Messages* instance_message(Messages new_message);
Messages* append_message(Messages* messages, Messages new_message);
Messages read_message(char* path);
int save_message(char* path, Messages message);
CMessages get_message_id(int target_id, int shm_id);
void reply_message(char* username, int socket, int shm_id);
void message_status(int socket, int shm_id);
void write_messgae(char* username, int socket, int shm_id);
void print_message_informations(int socket, int shm_id);

//SUBSCRIBING OPERATIONS
Subscribers* print_subscribers(char* path);
void subscribe(char* username, int socket, int shm_id);
Subscribers* instance_subscriber(char* subscriber);
int is_subscribed(int fd, char* username);
int add_subscriber(char* topic_name, char* username);
Subscribers* append_subscribers(Subscribers* subscribers, char* username);

//UTILITIES
void menu(int socket);
void get_input(char* username, char* password, int socket);
int get_id();
int increment_id();
void list(int socket, int shm_id);
int check_conn(int conn_status);
void map_whiteboard(int shmId, int socket);
void shm_clear();

#endif
