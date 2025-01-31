#include "game_player.h"

int to_server = 0;
int from_server = 0;

static void sighandler(int signo){
  if (signo == SIGINT){
    int forfeit = FORFEIT;
    int writeResult = write(to_server, &forfeit, sizeof(int));
    if (writeResult == -1) printf("big exiting error\n");
    printf("Forfeiting game.\n");
    sleep(10);
    close(to_server);
    close(from_server);
    exit(0);
  }
}

//Just needed a dummy handler
void sigalrm_handler(int s) {
    return;
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

int getPlayerInput(){
  // Creating timer
  timer_t clk;
  int realClock = timer_create(CLOCK_REALTIME, NULL, &clk);
  if (realClock < 0) err();

  struct sigaction sigalrm_act = {
   .sa_handler = sigalrm_handler,
   .sa_flags = 0
  };
  sigemptyset(&sigalrm_act.sa_mask);
  realClock = sigaction(SIGALRM, &sigalrm_act, NULL);
  if (realClock < 0) {
    err();
  }

  timer_settime(clk, 0, &seven_second, NULL);

  char numberInputted[20];
  if (! fgets(numberInputted, 20, stdin)){
    if (errno == EINTR){
      printf("Sorry you ran out of time. You lose.\n");
      int timeLoss = TIMELOSS;
      int writeResult = write(to_server, &timeLoss, sizeof(int));
      if (writeResult == -1) err();
      sleep(2);
      close(to_server);
      close(from_server);
      exit(0);
    }
    else {
      err();
    }
  }

  timer_settime(clk, 0, &stop_timer, NULL);

  int answer = stringToNum(numberInputted, strlen(numberInputted));
  return answer;
}

int main() {
  signal(SIGINT, sighandler);

  char* nextRoundStr = "***************************************\n";

  //printf("y u no work client\n");

  from_server = client_handshake( &to_server );

  //Writing PID to client
  int pid = getpid();
  int writeResult = write(to_server, &pid, sizeof(int));
  if (writeResult == -1) err();
    printf("Waiting for round to begin...\n");

  while (1){
    int numbers[2];
    int readResult = read(from_server, numbers, 2*sizeof(int));
    if (readResult == -1) err();

    // Checking BYE
    if (numbers[0] == BYE){
      printf("You have a bye! Wait for your next round.\n%s", nextRoundStr);
      // Rewriting pid to client
      writeResult = write(to_server, &pid, sizeof(int));
      if (writeResult == -1) err();
    printf("Waiting for round to begin...\n");
      continue;
    }

    // Checking if the game has ended
    if (numbers[0] < 0){
      if (numbers[0] == VICTORY){
        printf("CONGRATULATIONS! Your opponent failed, and you win this round. Wait until your next round begins...\n%s", nextRoundStr);
        // Rewriting pid to client
        writeResult = write(to_server, &pid, sizeof(int));
        if (writeResult == -1) err();
    printf("Waiting for round to begin...\n");
        continue;
      }
      else if (numbers[0] == LOSS){
        printf("Sorry, your answer was incorrect. You lose.\n");
        close(from_server);
        close(to_server);
        exit(0);
      }
      else if (numbers[0] == ULTIMATEVICTORY){
        printf("%sCONGRATS! You are the ULTIMATE victor. Brag about your math skills to all of your friends.\nWait a minute. You're a math whiz and probably a CS nerd too if you're running this program. Statistically, you don't have many friends. :(\nWell, you can always brag about it to the internet, where nobody cares.\n", nextRoundStr);
        close(from_server);
        close(to_server);
        exit(0);
      }
    }

    // Taking answer from player and turning it into an int
    printf("Your task: add %d to %d\n", numbers[0], numbers[1]);
    int answer = getPlayerInput();

    // Writing answer to the server
    printf("Answer Inputted: %d\n", answer);
    writeResult = write(to_server, &answer, sizeof(int));
    if (writeResult == -1) err();
  }
}
