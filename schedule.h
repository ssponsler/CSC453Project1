#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAX_PROCESSES 100
#define MAX_ARGUMENTS 10