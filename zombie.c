#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t parent_pid, child_pid;

    parent_pid = getpid(); // Get parent process ID

    // Create a child process
    child_pid = fork();

    if (child_pid > 0) {
        // Parent process
        printf("Parent process (PID: %d) is sleeping for 5 minutes...\n", parent_pid);
        sleep(300); // Sleep for 5 minutes (300 seconds) to give time for the child process to become a zombie
        printf("Parent process (PID: %d) exiting.\n", parent_pid);
    } else if (child_pid == 0) {
        // Child process
        child_pid = getpid(); // Get child process ID
        printf("Child process (PID: %d) is exiting.\n", child_pid);
        exit(0);
    } else {
        // Error occurred
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return 0;
}
