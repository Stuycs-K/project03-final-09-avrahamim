#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#define MAX_PLAYERS 100
#define READ 0
#define WRITE 1
#define GENPIPE "generalServerPipe"
#define TOTALPLAYERS 3
#define DOWN -1
#define UP 1
#define VICTORY -10 //Randomly chosen numbers that cannot
#define LOSS -20    //possibly match any game numbers
#define TIMELOSS -30
#define ULTIMATEVICTORY -40
#define BYE -50

union semun{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
  struct seminfo *__buf;
};

struct itimerspec seven_second = {
  .it_interval = {.tv_sec = 0, .tv_nsec = 0},
  .it_value = {.tv_sec = 7, .tv_nsec = 0}
};

struct itimerspec stop_timer = {
  .it_interval = {.tv_sec = 0, .tv_nsec = 0},
  .it_value = {.tv_sec = 0, .tv_nsec = 0}
};
