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

int wonOrLost(int shmid){
  int * data;
  data = shmat(shmid, 0, 0);
  int result = *data;
  shmdt(data);
  return result;
}

int main() {
  signal(SIGINT, sighandler);
  int to_server;
  int from_server;
  //printf("y u no work client\n");

  from_server = client_handshake( &to_server );

  // Accessing shared memory to tell if won or lost
  int shmkey;
  int readResult = read(from_server, &shmkey, sizeof(int));
  if (readResult == -1) err();

  int shmid = shmget(shmkey, sizeof(int), IPC_CREAT | 0640);

  while (1){
    // Checking if the game has ended
    int result = wonOrLost(shmid);
    if (result){
      if (result == VICTORY){
    	printf("CONGRATULATIONS! Your opponent failed, and you win this round.\n");
      }
      else if (result == LOSS){
    	printf("Sorry you ran out of time, or your answer was incorrect. You lose.\n");
      }
      close(to_server);
      close(from_server);
      exit(0);
    }

    int numbers[2];
    readResult = read(from_server, numbers, 2*sizeof(int));
    if (readResult == -1) err();

    // Taking answer from player and turning it into an int
    printf("Your task: add %d to %d\n", numbers[0], numbers[1]);
    char numberInputted[20];
    fgets(numberInputted, 20, stdin);
    int answer = stringToNum(numberInputted, 20);

    // Writing answer to the server
    printf("answer inputted: %d\n", answer);
    int writeResult = write(to_server, &answer, sizeof(int));
    if (writeResult == -1) err();
  }
}
