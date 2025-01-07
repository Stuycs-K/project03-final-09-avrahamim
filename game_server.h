#include "pipe_networking.h"
#include <signal.h>
#include <math.h>
#include <errno.h>
#define MAX_PLAYERS 100
#define READ 0
#define WRITE 1
#define GENPIPE "generalServerPipe"
