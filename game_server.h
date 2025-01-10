#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#define MAX_PLAYERS 100
#define READ 0
#define WRITE 1
#define GENPIPE "generalServerPipe"
#define TOTALPLAYERS 3
