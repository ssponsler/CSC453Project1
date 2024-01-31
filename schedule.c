#include "schedule.h"

int numProcesses = 0;
// global queue to hold processes
pid_t processes[MAX_PROCESSES];
int quantum;
int currentProcess = 0;
struct itimerval timer;

// queue to store order of original process list
// important for proper round-robin handling
// without this, just becomes strict-FIFO
char* originalProcesses[MAX_PROCESSES];
// queue iterators
int front = 0;
int rear = -1;

// function to enqueue a process into the queue
void enqueue(char* process) {
    rear++;
    originalProcesses[rear] = process;
}

// function to dequeue a process from the queue
char* dequeue() {
    if (front > rear) {
        return NULL; // queue is empty
    }
    char* process = originalProcesses[front];
    front++;
    return process;
}

// function to execute the child process
void executeChild(char *command, char *arguments[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) { // child process
        // stop the child process immediately
        raise(SIGSTOP);
        execv(command, arguments);
        perror("execv");
        exit(1);
    } else { // parent process
        // add the child process to the queue
        enqueue(command);
        processes[numProcesses++] = pid;
    }
}

// signal handler for the timer expiration
void timerHandler(int signum) {
    // pause the current process
    kill(processes[currentProcess], SIGSTOP);

    // get the next original process from the queue
    char* nextProcess = dequeue();

    if (nextProcess != NULL) {
        int i;
        // ensure next process is authentic original process
        for (i = 0; i < numProcesses; i++) {
            if (strcmp(nextProcess, originalProcesses[i]) == 0) {
                currentProcess = i;
                break;
            }
        }
    } else {
        // no more processes in the queue, wrap around to repeat
        // to satisfy round-robin
        currentProcess = (currentProcess + 1) % numProcesses;
    }

    // resume the next process
    kill(processes[currentProcess], SIGCONT);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s quantum [prog1 [args] [: prog2 [args] [: ...]]]\n", argv[0]);
        exit(1);
    }
    
    // convert quantum to usable long
    char *endptr;
    quantum = strtol(argv[1], &endptr, 10);
    // validation
    if (*endptr != '\0' && *endptr != '\n') {
       fprintf(stderr, "Invalid quantum value: %s\n", argv[1]);
       exit(1);
    }

    // set up the timer
    memset(&timer, 0, sizeof(timer));
    // microseconds to milliseconds
    timer.it_value.tv_usec = quantum * 1000;
    timer.it_interval.tv_usec = quantum * 1000;
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, timerHandler);

    // start the processes and enqueue original processes
    // all done in a single parent process for management
    int i = 2;
    while (i < argc) {
        // skip colon symbol
        if (strcmp(argv[i], ":") == 0) {
            i++;
            continue;
        }

        char *command = argv[i];
        // account for 2 required arguments
        // process name + NULL
        char *arguments[MAX_ARGUMENTS + 2];
        // first argument for execv() is process name
        arguments[0] = command;

        int j = 1;
        i++; // move to the first argument after the command
        // until colon, append arguments
        while (i < argc && strcmp(argv[i], ":") != 0 && j <= MAX_ARGUMENTS) {
            arguments[j] = argv[i];
            i++;
            j++;
        }
        // set final argument to NULL to satisfy execv()
        arguments[j] = NULL;

        executeChild(command, arguments);
    }

    // allow the first process to start
    if (numProcesses > 0) {
        kill(processes[currentProcess], SIGCONT);
    }

    // wait for child processes to complete
    while (numProcesses > 0) {
        int status;
        // wait for child processes to stop or terminate
        waitpid(-1, &status, WUNTRACED);
        
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            // child process has terminated or received a signal
            // proceed to the next process
            for (i = currentProcess; i < numProcesses - 1; i++) {
                processes[i] = processes[i + 1];
            }
            // decrement # of processes
            numProcesses--;

            if (numProcesses > 0) {
                currentProcess = currentProcess % numProcesses;
                // resume the next process
                kill(processes[currentProcess], SIGCONT);
            }
        }
    }

    return 0;
}
