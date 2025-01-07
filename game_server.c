#include "game_server.h"
static void sighandler(int signo){
  if (signo == SIGINT){
    remove(WKP);
    exit(0);
  }
  if (signo == SIGPIPE){}
}

// Returns the pid of whichever player won the round

void runGame(){
  int to_client;
  int from_client;
  int numPlayers = 0;
  int* players = (int*)(calloc(MAX_PLAYERS, sizeof(int)));

  while (1){
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
    if (!forkResult){
      to_client = subserver_connect( from_client );

      char line[BUFFER_SIZE];

      while (1){
        int readResult = read(from_client, line, BUFFER_SIZE);
        if (readResult == -1) { printf("reading from client string failed\n"); exit(1);}
        //Do something to line
        int writeResult = write(to_client, line, BUFFER_SIZE);
        if (writeResult == -1) { printf("Client closed connection.\n"); break;}
        sleep(1);
      }
      close(from_client);
      close(to_client);
      exit(0);
    }
  }
}

int main() {
  signal(SIGINT, sighandler);
  signal(SIGPIPE, sighandler);
  printf("parent pid: %d\n", getpid());

  runGame();
  return 0;
}
