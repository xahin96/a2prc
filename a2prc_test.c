#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // Include for getuid
#include <signal.h> // Include for kill function

#define MAX_PIDS 100 // Maximum number of PIDs to store
#define BUFFER_SIZE 1024

int foundPIDs[MAX_PIDS]; // Global array to store found PIDs
int numFound = 0; // Number of PIDs found

// Function to kill a process if it was created by the current user
void killProcessIfCreatedByMe(pid_t pid) {
    // Get the user ID of the current process
    uid_t my_uid = getuid();

    // Get the real user ID of the process to check
    uid_t process_uid;
    FILE *fp;
    char filename[100];
    char line[100];
    sprintf(filename, "/proc/%d/status", pid);
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }
    while (fgets(line, 100, fp) != NULL) {
        if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line, "Uid: %d", &process_uid);
            break;
        }
    }
    fclose(fp);

    // Check if the process was created by the current user
    if (process_uid == my_uid) {
        printf("The process with PID %d was created by you.\n", pid);

        // Kill the process
        if (kill(pid, SIGKILL) == 0) {
            printf("Process killed successfully.\n");
        } else {
            perror("Error killing process");
        }
    } else {
        printf("The process with PID %d was not created by you.\n", pid);
    }
}

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
                // Store the child PID in the global array
                foundPIDs[numFound++] = pid;
                pclose(pipe);
                return true;
            }
                // If the parent PID does not match, recursively search for the parent process
            else {
                bool found = searchChildProcess(ppid, parentPID);
                if (found) {
                    // Store the child PID in the global array
                    foundPIDs[numFound++] = pid;
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
    argc = 4;
    argv[1] = "44455";
    argv[2] = "44450";
    argv[3] = "-pr";
    int childPID = atoi(argv[1]);
    int parentPID = atoi(argv[2]);

    if (!searchChildProcess(childPID, parentPID)) {
        printf("Child process %d not found under parent process %d\n", childPID, parentPID);
    }

    // Use the foundPIDs array in the if blocks as needed
    if (argc == 3) {
        // Print the found PIDs
        printf("Found PIDs: ");
        for (int i = 0; i < numFound; ++i) {
            printf("%d ", foundPIDs[i]);
        }
        printf("\n");
        return 0;
    }
    if (argc == 4) {
        if (strcmp(argv[3], "-rp") == 0)
        {
            // Kill the processes found in the foundPIDs array
            for (int i = 0; i < numFound; ++i) {
                if (kill(foundPIDs[i], SIGKILL) == 0) {
                    printf("Process %d killed\n", foundPIDs[i]);
                } else {
                    printf("Failed to kill process %d\n", foundPIDs[i]);
                }
            }
        }
        if (strcmp(argv[3], "-pr") == 0)
        {
            killProcessIfCreatedByMe(parentPID);
            return 0;
        }
    }
    printf("Usage: %s <childPID> <parentPID>\n", argv[0]);
    return 0;
}
