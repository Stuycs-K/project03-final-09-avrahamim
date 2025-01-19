#include "game_server.h"
static void sighandler(int signo){
  if (signo == SIGINT){
    remove(WKP);
    exit(0);
  }
  if (signo == SIGPIPE){}
}

void err(){
  printf("[%d] Errno: %d\n", getpid(), errno);
  printf("Error: %s\n", strerror(errno));
  exit(0);
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

  int semid = semget(randNum, 1, IPC_CREAT | 0666);

  union semun us;
  us.val = 1;
  semctl(semid, 0, SETVAL, us);

  printf("semkey: %u\n", randNum);

  return randNum;
}

// Creates shared memory integer and returns the key
int createSharedInt(){
  int randNum = getRandomNumber();

  int shmid = shmget(randNum, sizeof(int), IPC_CREAT | 0666);
  printf("shmid: %d\n", shmid);

  return shmid;
}

// Checks if the given pid is still alive in the game
int stillAlive(int pid, int* pidsThatDied, int lenList){
  for (int i = 0; i < lenList; i++){
  	if (pid == *(pidsThatDied + i)){
  		return 0;
  	}
  }
  return 1;
}

// Makes a new pid list with only the remaining players
int* getNewPlayerList(int* players, int* playersThatDied, int lenList){
  int newCount = 0;
  int* newPlayers = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  for (int i = 0; i < lenList; i++){
    if (stillAlive(*(players + i), playersThatDied, lenList)){
      *(newPlayers + newCount) = *(players + i);
      newCount++;
    }
  }
  return newPlayers;
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

// Subserver takes the pastNumber, and interacts with the client to return the summed number
int playerAdd(int playerPID, int from_client, int to_client, int pastNum){

  // Writing numbers to add to client
  int randNum = getRandomNumber() % 200;
  int numbers[2];
  numbers[0] = pastNum;
  numbers[1] = randNum;
  int writeResult = write(to_client, numbers, 2*sizeof(int));
  if (writeResult == -1) err();

  // Reading the response from the client
  int answer;
  int readResult = read(from_client, &answer, sizeof(int));
  if (readResult == -1){
    err();
  }

  // Returns the value if the player answered correctly, and the type of loss if not
  if (answer == pastNum + randNum){
    printf("[%d] %d is correct!\n",getpid(), answer);
    return answer;
  }
  else if (answer == TIMELOSS){
    printf("[%d] client timed out.\n", getpid());
    return TIMELOSS;
  }
  else {
    printf("[%d] %d is incorrect.\n", getpid(), answer);
    return LOSS;
  }
}

// This is where the individual games take place. Returns whether this player/subserver combination won against their opponent (1) or lost (0)
int playGame(int from_client, int to_client, int subserverID, int genPipeFd){
  if (subserverID == ULTIMATEVICTORY){
    // Reading PID from client to free it to read its result
    int playerPID;
    int readResult = read(from_client, &playerPID, sizeof(int));
    if (readResult == -1) err();

    int ultVictories[] = {ULTIMATEVICTORY, ULTIMATEVICTORY};
    int writeResult = write(to_client, ultVictories, 2*sizeof(int));
    if (writeResult == -1) err();
    sleep(2);
    return -1;
  }

  sleep(2);


  // Grabbing shared memory and semaphore info
  int* sharedInts = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int readResult = read(genPipeFd, sharedInts, MAX_PLAYERS * sizeof(int));
  if (readResult == -1) err();

  printf("[%d] past reading shm\n", getpid());

  sleep(2);

  int* semaphores = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  readResult = read(genPipeFd, semaphores, MAX_PLAYERS * sizeof(int));
  if (readResult == -1) err();

  printf("[%d] past reading sem\n", getpid());

  int sharedIntKey = *(sharedInts + subserverID);
  int semaphoreKey = *(semaphores + subserverID);
  printf("[%d]. Shared memory: %u. Semaphore: %u\n", getpid(), sharedIntKey, semaphoreKey);

  // Accessing semaphore and shared memory
  int shmid = shmget(sharedIntKey, sizeof(int), IPC_CREAT | 0666);
  if (shmid == -1) err();

  int semid = semget(semaphoreKey, 1, 0);
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = DOWN;

  // Attaching variable to shared memory
  int * data = 0;
  data = shmat(shmid, 0, 0);

 // Reading PID from client
  int playerPID = 0;
  readResult = read(from_client, &playerPID, sizeof(int));
  if (readResult == -1) err();

  // Byes automatically advance
  if (*(sharedInts + subserverID) == 0){
    int toClient[] = {BYE, BYE};
    int writeResult = write(to_client, toClient, 2*sizeof(int));
    if (writeResult == -1) err();
    sleep(2);
    return 1;
  }

  sleep(2);

  // Begins the game by downing the semaphore, accessing the client answer, then upping the semaphore
  while (1){
    sb.sem_op = DOWN;
    semop(semid, &sb, 1);

    printf("[%d] downed the semaphore\n", getpid());

    // Check if player won
    if (*data == VICTORY){
      printf("[%d] won\n", getpid());
      int victory[] = {VICTORY, VICTORY};
      int writeResult = write(to_client, victory, 2*sizeof(int));
      if (writeResult == -1) err();
      return 1;
    }
    // if result is positive, the player answered correctly. If result == -1, the player did not
    // answer correctly and they return -1, and set *data = -1 to let opponent know they won
    int result = playerAdd(playerPID, from_client, to_client, *data);

    *data = result;
    printf("[%d] result: %d\n",getpid(), result);
    if (result == TIMELOSS){
      *data = VICTORY;
      printf("[%d] timeloss\n", getpid());
      sb.sem_op = UP;
      semop(semid, &sb, 1);
      return -1;
    }
    if (result == LOSS){
      // Turns sharedInt to victory so opponent knows they've won
      *data = VICTORY;
      printf("[%d] someone lost\n", getpid());
      // Tell player they lost
      int loss[] = {LOSS, LOSS};
      int writeResult = write(to_client, loss, 2*sizeof(int));
      if (writeResult == -1) err();
      // Up the semaphore so the opponent can access their victory
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
void gameHub(int numPlayers, int* players, int genPipeFd){
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

  // Assigning shared memory and semaphore to players, or bye
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

  sleep(2);

// Writing shared memory and semaphores to all subservers
  for (int i = 0; i < numPlayers; i++){
    printf("[%d] SharedMemoryKey: %u. SemaphoreKey: %u\n", getpid(), *(sharedInts + i), *(semaphores + i));
  }

  for (int i = 0; i < numPlayers; i++){
    int writeResult = write(genPipeFd, sharedInts, MAX_PLAYERS * sizeof(int));
    if (writeResult == -1) err();
  }


  for (int i = 0; i < numPlayers; i++){
    int writeResult = write(genPipeFd, semaphores, MAX_PLAYERS * sizeof(int));
    if (writeResult == -1) err();
  }


  // Waiting for all the games to finish
  int returnPIDS[numPlayers / 2];

  for (int i = 0; i < numPlayers / 2; i++){
    int statusThing = 0;
    int* status = &statusThing;
    int returnPID = wait(status);
    returnPIDS[i] = returnPID;
    printf("returnPID: %d\n", returnPID);
  }

  printf("finished waiting on children to return\n");

  // Giving all subservers new subserverIDs to connect to the next game
  int newNumPlayers = (numPlayers + 1) / 2;

  int *newSubserverIDS = (int*)calloc(MAX_PLAYERS, sizeof(int));
  int count = 0;
  for (int i = 0; i < numPlayers; i++){
  	if (stillAlive(*(players + i), returnPIDS, newNumPlayers)){
  		*(newSubserverIDS+i) = count;
  		count++;
  	}
  	else {
  		*(newSubserverIDS+i) = -1;
  	}
  }

  int* newPlayers = getNewPlayerList(players, returnPIDS, numPlayers);

  // printf("count: %d and newNumPlayers: %d (should be equal)\n", count, newNumPlayers);
  // for (int i = 0; i < numPlayers; i++){
  	// printf("pid: %d newID: %d\n", *(players + i), *(newSubserverIDS + i));
  // }

  // Writing new subserverIDs to all winners

  sleep(2);

  for (int i = 0; i < newNumPlayers; i++){
      printf("player %d still alive\n", *(newPlayers + i));
      // If game ended
      if (newNumPlayers <= 1){
        for (int j = 0; j < numPlayers; j++){
          *(newSubserverIDS + j) = ULTIMATEVICTORY;
        }
      }
      int writeResult = write(genPipeFd, newSubserverIDS, MAX_PLAYERS * sizeof(int));
      if (writeResult == -1) err();
    }

   printf("count: %d and newNumPlayers: %d (should be equal)\n", count, newNumPlayers);
   for (int i = 0; i < numPlayers; i++){
         printf("pid: %d newID: %d\n", *(players + i), *(newSubserverIDS + i));
   }

// If game has ended
   if (newNumPlayers <= 1){
     sleep(2);
     return;
   }

   sleep(3);


   gameHub(newNumPlayers, newPlayers, genPipeFd);
}

void initializeGame(){
  int to_client;
  int from_client;
  int numPlayers = 0;
  int* players = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
  int subserverID;

  mkfifo(GENPIPE, 0666);
  chmod(GENPIPE, 0666);

  char numPlayersEntered[100];
  printf("Enter the number of players that are going to play: ");
  fgets(numPlayersEntered, 100, stdin);
  int totalPlayers = stringToNum(numPlayersEntered, strlen(numPlayersEntered));


  while (numPlayers < totalPlayers){
    // Waiting for client to connect to server
    printf("[%d] awaiting %d players:\n", getpid(), totalPlayers - numPlayers);
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
      printf("[%d] subserverID: %d\n", getpid(), subserverID);
      to_client = subserver_connect( from_client );

      int genPipeFd = open(GENPIPE, O_RDONLY);
      if (genPipeFd == -1) err();
      while (1){
        int result = playGame(from_client, to_client, subserverID, genPipeFd);
        if (result == -1) break;
        int * subserverIDS = (int*)(calloc(MAX_PLAYERS, sizeof(int)));
        printf("got here\n");
        int readResult = read(genPipeFd, subserverIDS,MAX_PLAYERS * sizeof(int));
        subserverID = *(subserverIDS + subserverID);
        if (readResult == -1) err();
        printf("[%d] new subserverID: %d\n", getpid(), subserverID);
      }
     // printf("someone is exiting\n");
      close(to_client);
      close(from_client);
      exit(0);
    }
  }

  // Only main server now. Begins the real :) gameplay
  int genPipeFd = open(GENPIPE, O_WRONLY);
  if (genPipeFd == -1) err();

  gameHub(numPlayers, players, genPipeFd);
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
