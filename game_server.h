#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#define MAX_PLAYERS 100
#define READ 0
#define WRITE 1
#define GENPIPE "generalServerPipe"
#define TOTALPLAYERS 2
#define DOWN -1
#define UP 1
#define VICTORY -10 //Randomly chosen numbers that cannot
#define LOSS -20    //possibly match any game numbers

union semun{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
  struct seminfo *__buf;
};
