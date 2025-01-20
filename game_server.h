#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>

#ifndef SERVER_H
#define SERVER_H
#define MAX_PLAYERS 100
#define READ 0
#define WRITE 1
#define GENPIPE "generalServerPipe"
#define TOTALPLAYERS 2
#define DOWN -1
#define UP 1
#define VICTORY -10 //Randomly chosen numbers that cannot
#define LOSS -20    //possibly match any game numbers
#define TIMELOSS -30 //
#define ULTIMATEVICTORY -40 //
#define BYE -50 //
#define FORFEIT -60 //

union semun{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
  struct seminfo *__buf;
};

int getRandomNumber();
int createSemaphore();
int createSharedInt();

int stillAlive(int pid, int* pidsThatDied, int lenList);
int* getNewPlayerList(int* players, int* playersThatDied, int lenList);
int stringToNum(char* string, int size);

int playerAdd(int playerPID, int from_client, int to_client, int pastNum);
void gameHub(int numPlayers, int* players, int genPipeFd);
void initializeGame();

#endif
