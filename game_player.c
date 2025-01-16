#include "game_player.h"

static void sighandler(int signo){
  if (signo == SIGINT){
    exit(0);
  }
  if (signo == SIGQUIT){
    printf("Sorry, you ran out of time. You lose.\n");
    exit(0);
  }
}

void err(){
  printf("Errno: %d\n", errno);
  printf("Error: %s\n", strerror(errno));
  exit(0);
}

int stringToNum(char* string, int size){
  char* num = (char*)(calloc(size, sizeof(char)));
  int numIndex = 0;

  for (int i = 0; i < size; i++){
    if (*(string + i) >= '0' && *(string + i) <= '9'){
      *(num + numIndex) = *(string + i);
      numIndex++;
    }
  }

  return (atoi(num));
}

int main() {
  signal(SIGINT, sighandler);
  signal(SIGQUIT, sighandler);
  signal(SIGTERM, sighandler);

  printf("[%d]\n", getpid());
  int to_server;
  int from_server;
  //printf("y u no work client\n");

  from_server = client_handshake( &to_server );

  //Writing PID to client
  int pid = getpid();
  int writeResult = write(to_server, &pid, sizeof(int));
  if (writeResult == -1) err();

  while (1){
    int numbers[2];
    int readResult = read(from_server, numbers, 2*sizeof(int));
    if (readResult == -1) err();

    // Checking if the game has ended
    printf("ended?: %d\n", numbers[0]);
    if (numbers[0] == LOSS || numbers[0] == VICTORY ){
      if (numbers[0] == VICTORY){
        printf("CONGRATULATIONS! Your opponent failed, and you win this round. Wait until your next round begins...\n");
        continue;
      }
      else if (numbers[0] == LOSS){
        printf("Sorry, your answer was incorrect. You lose.\n");
        close(from_server);
        close(to_server);
        exit(0);
      }
    }

    // Taking answer from player and turning it into an int
   printf("Your task: add %d to %d\n", numbers[0], numbers[1]);
    char numberInputted[20];
    fgets(numberInputted, 20, stdin);
    int answer = stringToNum(numberInputted, 20);

    // Writing answer to the server
    printf("answer inputted: %d\n", answer);
    writeResult = write(to_server, &answer, sizeof(int));
    if (writeResult == -1) err();
  }
}
