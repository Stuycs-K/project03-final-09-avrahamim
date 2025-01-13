#include "game_player.h"

static void sighandler(int signo){
  if (signo == SIGINT){
    exit(0);
  }
}

void err(){
  printf("Errno: %d\n", errno);
  printf("Error: %s\n", strerror(errno));
  exit(0);
}

int main() {
  signal(SIGINT, sighandler);
  int to_server;
  int from_server;
  //printf("y u no work client\n");

  from_server = client_handshake( &to_server );

  int numbers[2];
  int readResult = read(from_server, numbers, 2*sizeof(int));
  if (readResult == -1) err();

  printf("numbers[0]: %d. numbers[1]: %d\n", numbers[0], numbers[1]);

}
