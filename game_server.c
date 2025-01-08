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

// This is where the individual games take place. Returns whether this player/subserver combination won against their opponent (1) or lost (0)
int playGame(int from_client, int to_client, int subserverID){
  //printf("[%d] : %d\n", getpid(), subserverID);
  int genPipeFd = open(GENPIPE, O_RDONLY);
  if (genPipeFd == -1) err();

  // Grabbing opponent PID
  int* listOfOpponents = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int readResult = read(genPipeFd, listOfOpponents, MAX_PLAYERS * sizeof(int));
  if (readResult == -1) err();
  printf("readResult: %d\n", readResult);

  printf("[%d] opponent: %d\n", getpid(), *(listOfOpponents + subserverID));
  return 0;
}

// This is where the main server organizes the participants to play against each other, organizes results
void gameHub(int numPlayers, int* players){
  printf("[%d] numPlayers: %d\n", getpid(), numPlayers);
  int* opponents = (int*)(calloc(MAX_PLAYERS, sizeof(int)));

  // If there are an odd number of players, one random player gets a bye
  int byePlayerIndex = -1;
  if (numPlayers % 2 != 0){
    unsigned int randNum;
    int randfd = open("/dev/random", O_RDONLY);
    int readResult = read(randfd, &randNum, sizeof(int));
    if (readResult == -1) err();
    byePlayerIndex = randNum % numPlayers;
  }
  printf("[%d] byePlayerIndex: %d\n", getpid(), byePlayerIndex);

  for (int i = 0; i < numPlayers; i+=2){
    int nextPlayer = 1;
    if (i == byePlayerIndex){
      i--;
      continue;
    }
    if (i + 1 == byePlayerIndex){
      nextPlayer = 2;
    }

    *(opponents + i) = *(players + i + nextPlayer);
    *(opponents + i + nextPlayer) = *(players + i);

    if (i + 1 == byePlayerIndex){
      i++;
    }
  }

  for (int i = 0; i < numPlayers; i++){
    printf("[%d] Player: %d. Opponent: %d\n", i, *(players + i), *(opponents + i));
  }

  int genPipeFd = open(GENPIPE, O_WRONLY);
  if (genPipeFd == -1) err();

  int writeResult = write(genPipeFd, opponents, MAX_PLAYERS * sizeof(int));
  if (writeResult == -1) err();

  sleep(3);
}

void initializeGame(){
  int to_client;
  int from_client;
  int numPlayers = 0;
  int* players = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int subserverID;

  mkfifo(GENPIPE, 0644);

  while (numPlayers < TOTALPLAYERS){
    // Waiting for client to connect to server
    printf("[%d] awaiting %d players:\n", getpid(), TOTALPLAYERS - numPlayers);
    from_client = server_setup();
    signed int forkResult = fork();

    // Parent process. Add child to numPlayers, continue waiting for clients
    if (forkResult){
      *(players + numPlayers) = forkResult;
      numPlayers++;

      continue;
    }
    // Subserver process. Finish connecting subserver to client, send subserver and client off to interact
    if (!forkResult){
      subserverID = numPlayers;
      to_client = subserver_connect( from_client );

      int result = playGame(from_client, to_client, subserverID);
      close(to_client);
      close(from_client);
      exit(0);
    }
  }

  // Only main server now. Begins the real :) gameplay
  gameHub(numPlayers, players);
}

int main() {
  signal(SIGINT, sighandler);
  signal(SIGPIPE, sighandler);
  printf("parent pid: %d\n", getpid());

  initializeGame();
  return 0;
}
