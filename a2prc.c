#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

// Function to recursively search for the child process under the specified parent process
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
                // If the parent PID does not match, recursively search for the parent process
            else {
                bool found = searchChildProcess(ppid, parentPID);
                if (found) {
                    // Print the child PID
                    printf("%d\n", pid);
                    pclose(pipe);
                    return true;
                }
            }
        }
    }
    // Close the pipe
    pclose(pipe);
    return false;
}

int main(int argc, char *argv[]) {
    // Check if the user provided the parent and child PIDs
    argc = 3;
    argv[1] = "9197";
    argv[2] = "9117";
    argv[3] = "-zs";
    if (argc == 3) {
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
    if (argc == 4) {
        if (strcmp(argv[3], "-rp") == 0)
        {
            printf("-rp process_id is killed if it belongs to the process tree rooted at root_process ");
        }
        if (strcmp(argv[3], "-pr") == 0)
        {
            printf("-pr the root_process is killed (if it is valid) ");
        }
        if (strcmp(argv[3], "-xn") == 0)
        {
            printf("-xn lists the PIDs of all the non-direct descendants of process_id ");
        }
        if (strcmp(argv[3], "-xd") == 0)
        {
            printf("-xd lists the PIDs of all the immediate descendants of process_id ");
        }
        if (strcmp(argv[3], "-xs") == 0)
        {
            printf(" -xs lists the PIDs of all the sibling processes of process_id ");
        }
        if (strcmp(argv[3], "-xt") == 0)
        {
            printf(" -xt process_id is paused with SIGSTOP");
        }
        if (strcmp(argv[3], "-xc") == 0)
        {
            printf(" -xc SIGCONT is sent to all processes that have been paused earlier ");
        }
        if (strcmp(argv[3], "-xz") == 0)
        {
            printf("-xz Lists the PIDs of all descendents of process_id that are defunct");
        }
        if (strcmp(argv[3], "-xg") == 0)
        {
            printf("-xg lists the PIDs of all the grandchildren of process_id");
        }
        if (strcmp(argv[3], "-zs") == 0)
        {
            printf("-zs prints the status of process_id (Defunct/ Not Defunct) ");
        }
        return 0;
    }
    printf("Usage: %s <childPID> <parentPID>\n", argv[0]);
    return 0;
}
