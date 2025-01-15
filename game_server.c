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

//Just needed a dummy handler
void sigalrm_handler(int s) {
    return;
}

int getRandomNumber(){
  unsigned int randNum;
  int randfd = open("/dev/random", O_RDONLY);
  int readResult = read(randfd, &randNum, sizeof(int));
  if (readResult == -1) err();

  int returnNum = randNum / 2;
  return returnNum;
}

// Creates a semaphore, sets value to 1, returns id
int createSemaphore(){
  int randNum = getRandomNumber();

  int semid = semget(randNum, 1, IPC_CREAT | 0640);

  union semun us;
  us.val = 1;
  semctl(semid, 0, SETVAL, us);

  printf("semkey: %u\n", randNum);

  return randNum;
}

// Creates shared memory integer and returns the key
int createSharedInt(){
  int randNum = getRandomNumber();

  int shmid = shmget(randNum, sizeof(int), IPC_CREAT | 0640);
  printf("shmid: %d\n", shmid);

  return shmid;
}

// Subserver takes the pastNumber, and interacts with the client to return the summed number
int playerAdd(int from_client, int to_client, int pastNum){
  // Writing numbers to add to client
  int randNum = getRandomNumber() % 200;
  int numbers[2];
  numbers[0] = pastNum;
  numbers[1] = randNum;
  int writeResult = write(to_client, numbers, 2*sizeof(int));
  if (writeResult == -1) err();

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

  // Giving the client time to respond, then checking if it is the write answer.
  int answer;
  int readResult = read(from_client, &answer, sizeof(int));
  if (readResult == -1){
  	if (errno == EINTR){
  		printf("Ran out of time.\n");
  		return LOSS;
  	}
  	else {
  		err();
  	}
  }

  timer_settime(clk, 0, &stop_timer, NULL);

  // Returns the value if the player answered correctly, and -1 if not
  if (answer == pastNum + randNum){
    printf("%d is correct!\n", answer);
    return answer;
  }
  else {
    printf("%d is incorrect.\n", answer);
    return LOSS;
  }
}

// This is where the individual games take place. Returns whether this player/subserver combination won against their opponent (1) or lost (0)
int playGame(int from_client, int to_client, int subserverID){
  //printf("[%d] : %d\n", getpid(), subserverID);
  int genPipeFd = open(GENPIPE, O_RDONLY);
  if (genPipeFd == -1) err();

  // Grabbing shared memory and semaphore info
  int* sharedInts = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int readResult = read(genPipeFd, sharedInts, MAX_PLAYERS * sizeof(int));
  if (readResult == -1) err();

  sleep(1);

  // Byes automatically advance
  if (*(sharedInts + subserverID) == 0){
    return 1;
  }

  int* semaphores = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  readResult = read(genPipeFd, semaphores, MAX_PLAYERS * sizeof(int));
  if (readResult == -1) err();

  int sharedIntKey = *(sharedInts + subserverID);
  int semaphoreKey = *(semaphores + subserverID);
  //printf("[%d]. Shared memory: %u. Semaphore: %u\n", getpid(), sharedIntKey, semaphoreKey);

  // Accessing semaphore and shared memory
  int shmid = shmget(sharedIntKey, sizeof(int), IPC_CREAT | 0640);
  if (shmid == -1) err();

  int semid = semget(semaphoreKey, 1, 0);
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = DOWN;

  // Attaching variable to shared memory
  int * data = 0;
  data = shmat(shmid, 0, 0);

  // Creating shared memory segment to tell player when they win or lose
  int *winOrLossData = 0;
  int shmkey = getRandomNumber();
  printf("shmkey: %d\n", shmkey);
  int thisshmid = shmget(shmkey, sizeof(int), IPC_CREAT | 0640);
  if (thisshmid == -1) err();

  winOrLossData = shmat(thisshmid, 0, 0);
  *winOrLossData = 0;

  // Writing shmkey to client so they know how to access the shared memory
  int thisWriteResult = write(to_client, &shmkey, sizeof(int));
  if (thisWriteResult == -1) err();

  // Begins the game by downing the semaphore, accessing the client answer, then upping the semaphore
  while (1){
    sb.sem_op = DOWN;
    semop(semid, &sb, 1);

    // Check if player won
    if (*data == VICTORY){
      printf("someone won\n");
      *winOrLossData = VICTORY;
      return 1;
    }
    // if result is positive, the player answered correctly. If result == -1, the player did not
    // answer correctly and they return -1, and set *data = -1 to let opponent know they won
    int result = playerAdd(from_client, to_client, *data);

    *data = result;
    printf("result: %d\n", result);
    if (result == LOSS){
      // Turns sharedInt to victory so opponent knows they've won
      *data = VICTORY;
      printf("someone lost\n");
      // Tell player they lost
      *winOrLossData = LOSS;
      // Up the semaphore so the opponent can access their loss
      sb.sem_op = UP;
      semop(semid, &sb, 1);
      return -1;
    }
    sb.sem_op = UP;
    semop(semid, &sb, 1);
  }
 // This should never be reached
  return 0;
}

// This is where the main server organizes the participants to play against each other, organizes results
void gameHub(int numPlayers, int* players){
  printf("[%d] numPlayers: %d\n", getpid(), numPlayers);
  int* sharedInts = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int* semaphores = (int*)(calloc(MAX_PLAYERS, sizeof(int)));

  // If there are an odd number of players, one random player gets a bye
  int byePlayerIndex = -1;
  if (numPlayers % 2 != 0){
    int randNum = getRandomNumber() % numPlayers;
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

    // Creating shared memory and semaphore for each pair
    int sharedMemoryKey = createSharedInt();
    int semaphoreKey = createSemaphore();

    *(sharedInts + i) = sharedMemoryKey;
    *(sharedInts + i + nextPlayer) = sharedMemoryKey;
    *(semaphores + i) = semaphoreKey;
    *(semaphores + i + nextPlayer) = semaphoreKey;

    if (i + 1 == byePlayerIndex){
      i++;
    }
  }

  for (int i = 0; i < numPlayers; i++){
    printf("[%d] SharedMemoryKey: %u. SemaphoreKey: %u\n", getpid(), *(sharedInts + i), *(semaphores + i));
  }

  int genPipeFd = open(GENPIPE, O_WRONLY);
  if (genPipeFd == -1) err();

  for (int i = 0; i < numPlayers; i++){
    int writeResult = write(genPipeFd, sharedInts, MAX_PLAYERS * sizeof(int));
    if (writeResult == -1) err();
  }


  for (int i = 0; i < numPlayers; i++){
    int writeResult = write(genPipeFd, semaphores, MAX_PLAYERS * sizeof(int));
    if (writeResult == -1) err();
  }

  int status = 0;
  int* statusP = &status;
  wait(statusP);
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

      playGame(from_client, to_client, subserverID);
     // printf("someone is exiting\n");
      close(to_client);
      close(from_client);
      exit(0);
    }
  }

  // Only main server now. Begins the real :) gameplay
  gameHub(numPlayers, players);
 // printf("someone is exiting\n");
  return;
}

int main() {
  signal(SIGINT, sighandler);
  signal(SIGPIPE, sighandler);
  printf("parent pid: %d\n", getpid());

  initializeGame();
  return 0;
}
