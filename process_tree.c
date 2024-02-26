#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void createProcessTree(int parentPID, int numChildren, int levels) {
    if (levels == 0) {
        return;
    }
    for (int i = 0; i < numChildren; i++) {
        int childPID = fork();
        if (childPID < 0) {
            // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (childPID == 0) {
            // Child process
            printf("Parent %d -> PID=%d\n", parentPID, getpid());
            // Recursively create children
            createProcessTree(getpid(), numChildren, levels - 1);
            // Child process sleeps to stay alive
            sleep(300); // Sleep for 5 minutes (300 seconds)
            // Child process terminates after sleeping
            exit(EXIT_SUCCESS);
        }
    }
    // Parent process waits for all its children to complete
    for (int i = 0; i < numChildren; i++) {
        wait(NULL);
    }
}

int main() {
    // Start with the root parent process
    int rootPID = getpid();
    // Create the process tree
    createProcessTree(rootPID, 4, 3); // Change the second and third arguments to control the number of children and levels
    return 0;
}
