#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

// Function to search for the child process under the specified parent process without recursion
bool searchChildProcess(int childPID, int parentPID) {
    // Open the pipe to read the output of the ps command
    FILE *pipe = popen("ps -o pid,ppid -ax", "r");
    if (!pipe) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    // Read the output of the ps command line by line
    char line[BUFFER_SIZE];
    fgets(line, BUFFER_SIZE, pipe); // Read and discard the header line
    while (fgets(line, BUFFER_SIZE, pipe)) {
        // Extract the PID and PPID from the line
        int pid, ppid;
        sscanf(line, "%d %d", &pid, &ppid);
        // Check if the current process is the child process
        if (pid == childPID) {
            // Check if the parent PID matches the specified parent PID
            if (ppid == parentPID) {
                // Print the child PID
                printf("%d\n", pid);
                pclose(pipe);
                return true;
            }
            // If the parent PID does not match, continue searching
        }
    }
    // Close the pipe
    pclose(pipe);
    return false;
}

int main(int argc, char *argv[]) {
    // Check if the user provided the parent and child PIDs
    argc = 3;
    argv[1] = "85319";
    argv[2] = "85308";
    if (argc != 3) {
        printf("Usage: %s <childPID> <parentPID>\n", argv[0]);
        return 1;
    }

    // Get the parent and child PIDs from the command line arguments
    int childPID = atoi(argv[1]);
    int parentPID = atoi(argv[2]);

    // Check if the provided PIDs are valid
    if (childPID <= 0 || parentPID <= 0) {
        printf("Invalid PIDs\n");
        return 1;
    }

    // Search for the child process under the specified parent process
    if (!searchChildProcess(childPID, parentPID)) {
        printf("Child process %d not found under parent process %d\n", childPID, parentPID);
    }

    return 0;
}
