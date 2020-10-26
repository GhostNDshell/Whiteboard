#include "serverUtils.h"

void is_authenticated(int authenticationStatus, int socket){

  int connectionStatus;
  char authenticationRequestMessage[] = "\t\t\t (ツ)_/¯ You need to be authenticated in order to run this command!\n";
  //Restrict the operation to authenticated users only
  if (authenticationStatus == 0) {
    connectionStatus = write(socket, authenticationRequestMessage, BUF_SIZE);
    if(check_conn(connectionStatus) == 0) exit(0);
  }
}

int authentication(char* username, char* password,  int socket){

  int authenticationStatus = 0;
  int connectionStatus;

  authenticationStatus = check_credentials(username, password);
  if (authenticationStatus == 1) {
    authenticationStatus = 2;
    connectionStatus = write(socket, "\t\t\t <[^_^]> User Authenticated!\n", BUF_SIZE);
  }
  else if (authenticationStatus == 3) {
    write(socket, "\t\t\t (ツ)_/¯ Wrong username or password!\n",BUF_SIZE );
  }
  else write(socket, "\t\t\t (ツ)_/¯ Unknown user, register to the WHITEBOARD!\n", BUF_SIZE);

  return authenticationStatus;
}

int check_credentials(char* username, char* password) {


        int auth = 0;

        struct dirent *d;

        DIR *users = opendir("../users/");
        if (users == NULL) {
                perror("opendir");
        }

        while ((d = readdir(users)) != NULL) {
                if (d->d_type == DT_REG) { //Check if it is a file

                        if (strcasecmp(username, d->d_name) == 0) {

                                char path[PATH_SIZE];

                                sprintf(path, "../users/%s", d->d_name);

                                int fd = open(path, O_RDONLY);
                                if (fd < 0) perror("open");

                                char buffer[100];
                                int i = 0;

                                while (read(fd, &buffer[i], 1) != 0) {
                                        if (strcmp(&buffer[i], "\n") == 0) break;
                                        i++;
                                }
                                strtok(buffer, "\n");


                                if (strcmp (password, buffer) == 0) {
                                        auth = 1;
                                }
                                else auth = 3;

                                close(fd);

                        }

                }

        }


        closedir(users);


        return auth;
}
