#include "game_server.h"
static void sighandler(int signo){
  if (signo == SIGINT){
    remove(WKP);
    exit(0);
  }
  if (signo == SIGPIPE){}
}

void err(){
  printf("Errno: %d\n", errno);
  printf("Error: %s\n", strerror(errno));
  exit(0);
}

void initializeGame(){
  int to_client;
  int from_client;
  int numPlayers = 0;
  int* players = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int subserverID;
  int* opponents = (int*)(calloc(MAX_PLAYERS, sizeof(int)));

  mkfifo(GENPIPE, 0644);
  int time = 0;

  while (time < 20){
    // Waiting for client to connect to server
    printf("[%d] awaiting next client:\n", getpid());
    from_client = server_setup();
    signed int forkResult = fork();

    // Parent process. Add child to numPlayers, continue waiting for clients
    if (forkResult){
      *(players + numPlayers) = forkResult;
      numPlayers++;

      int* playersPrint = players;
      while (*playersPrint){
        printf("%d\n", *(playersPrint));
        playersPrint++;
      }
      continue;
    }
    // Subserver process. Finish connecting subserver to client, send subserver and client off to interact
    if (!forkResult){
      subserverID = numPlayers;
      to_client = subserver_connect( from_client );

      close(to_client);
      close(from_client);
      exit(0);
    }
  }
}

int main() {
  signal(SIGINT, sighandler);
  signal(SIGPIPE, sighandler);
  printf("parent pid: %d\n", getpid());

  initializeGame();
  return 0;
}
