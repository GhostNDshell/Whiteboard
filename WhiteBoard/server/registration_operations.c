#include "serverUtils.h"

int registration(char* username, char* password, int socket){
  int registrationStatus = 0;
  int connectionStatus;
  char path[PATH_SIZE];
  int fd;
  int nbytes;

  if (!registrationStatus == check_username_available(username, password, socket)) {
    if(!registrationStatus == check_validity(username, password, socket)){

      sprintf(path, "../users/%s", username);
      if ((fd = open(path, O_WRONLY | O_CREAT, 0777)) < 0){
        perror("create");
        return 0;
      }//End if

      //Write his password in the file
      if ((nbytes = write(fd, password, strlen(password))) < 0) {
        perror("write");
        return 0;
      }//End if

      close(fd);
      connectionStatus = write(socket, "\t\t\t <[^_^]> User successfuly Registered!\n", BUF_SIZE);
      printf("d[o_0]b User %s has been Successfuly registered!\n", username);

      registrationStatus = 1; //successful registration
      return registrationStatus;
    }
  }
}//End function

int check_username_available(char* username, char* password, int socket){
  int registrationStatus = 1; // Registration status (0- not registered / 1- registered)
  char path[PATH_SIZE];
  int connectionStatus;

  struct dirent *dr; //Directory entries

  //Check if the users directory exist
  DIR *users = opendir("../users/");
  if (users == NULL) {
          perror("opendir");
  }//End if

  /** Check if the username is already used,
    if not create a file for a new user with that name **/
  while ((dr = readdir(users)) != NULL) {
    if (dr->d_type == DT_REG) { //Check if dr is a file
      if (strcasecmp(username, dr->d_name) == 0) {
        sprintf(path, "\t\t\t (ツ)_/¯ Username -- %s -- already taken!\n", dr->d_name);
        connectionStatus = write(socket, path, BUF_SIZE);
        registrationStatus = 0; //Set the refistration status to 0
      }//End if
    }//End if
  }//End while
  closedir(users);
  return registrationStatus; // Return registrationStatus = 1 Successful or 0 failed
}//End function

int check_validity(char* username, char* password, int socket){

  //int valid = 1;
  int connectionStatus = 1;

  for(int i = 0; i < strlen(username); i++){
    if(!isalpha(username[i]) )
    {

      connectionStatus = write(socket, "\t\t\t(ツ)_/¯ Username must contain only alpha characters!\n", BUF_SIZE);
      break;
    }
  }

  for(int i = 0; i < strlen(password); i++){
    if(!isdigit(password[i]))
    {

      connectionStatus = write(socket, "\t\t\t(ツ)_/¯ Password must contain only numeric characters!\n", BUF_SIZE);
      break;
    }
  }

//  printf(">> valid = %d\n", valid);
  return connectionStatus;

}
