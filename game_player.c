#include "game_player.h"

static void sighandler(int signo){
  if (signo == SIGINT){
    exit(0);
  }
}

int main() {
  signal(SIGINT, sighandler);
  int to_server;
  int from_server;
  //printf("y u no work client\n");

  from_server = client_handshake( &to_server );

  char line[BUFFER_SIZE];
  char * lineToCopy = "LINE_TO_SEND_TO_SERVER";
  for (int i = 0; i < strlen(lineToCopy); i++){
    *(line + i) = *(lineToCopy + i);
  }
  printf("%s\n", line);
  while (1){
    int writeResult = write(to_server, line, BUFFER_SIZE);
    if (writeResult == -1){ printf("Writing string to server failed"); exit(1);}
    sleep(1);
    int readResult = read(from_server, line, BUFFER_SIZE);
    if (readResult == -1){ printf("Reading string from server failed"); exit(1);}
    printf("%s\n", line);
  }

}
