#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>

#ifndef PLAYER_H
#define PLAYER_H
#define VICTORY -10 // Randomly chosen numbers that cannot possibly
#define LOSS -20 // match any game numbers
#define TIMELOSS -30 //
#define ULTIMATEVICTORY -40 //
#define BYE -50 //
#define FORFEIT -60 //

struct itimerspec seven_second = {
  .it_interval = {.tv_sec = 0, .tv_nsec = 0},
  .it_value = {.tv_sec = 7, .tv_nsec = 0}
};

struct itimerspec stop_timer = {
  .it_interval = {.tv_sec = 0, .tv_nsec = 0},
  .it_value = {.tv_sec = 0, .tv_nsec = 0}
};

void sigalrm_handler(int s);
int stringToNum(char* string, int size);
int getPlayerInput();

#endif
