#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define MAX_PIDS 100 // Maximum number of PIDs to store
#define BUFFER_SIZE 1024

int foundPIDs[MAX_PIDS]; // Global array to store found PIDs
int numFound = 0;        // Number of PIDs found

// Function to check if a process was created by the current user
bool isProcessCreatedByMe(pid_t pid) {
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
        return false;
    }
    while (fgets(line, 100, fp) != NULL) {
        if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line, "Uid: %d", &process_uid);
            break;
        }
    }
    fclose(fp);

    // Check if the process was created by the current user
    return (process_uid == my_uid);
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

// Function to recursively search for non-direct descendants of a process
void searchNonDirectDescendants(int parentPID)
{
    numFound = 0;
    // Open the pipe to read the output of the ps command
    FILE *pipe = popen("ps -o pid,ppid -ax", "r");
    if (!pipe)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    // Read the output of the ps command line by line
    char line[BUFFER_SIZE];
    fgets(line, BUFFER_SIZE, pipe); // Read and discard the header line
    while (fgets(line, BUFFER_SIZE, pipe))
    {
        // Extract the PID and PPID from the line
        int pid, ppid;
        sscanf(line, "%d %d", &pid, &ppid);
        // Check if the current process is a non-direct descendant of the specified parent process
        if (ppid != parentPID && searchChildProcess(ppid, parentPID))
        {
            // Store the PID of the non-direct descendant
            foundPIDs[numFound++] = pid;
        }
    }
    // Close the pipe
    pclose(pipe);
}

void searchDirectDescendants(int parentPID) {
    // Clear the foundPIDs array before populating it with new direct descendants
    numFound = 0;

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

        // Check if the current process is a direct descendant of the specified parent process
        if (ppid == parentPID) {
            // Store the PID of the direct descendant at the end of the foundPIDs array
            foundPIDs[numFound++] = pid;
        }
    }
    // Close the pipe
    pclose(pipe);
}


// Function to get the parent process ID (ppid) of a given process ID (pid)
int getParentPID(int processID) {
    FILE *pipe = popen("ps -o pid,ppid -ax", "r");
    if (!pipe) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    fgets(line, BUFFER_SIZE, pipe); // Read and discard the header line
    while (fgets(line, BUFFER_SIZE, pipe)) {
        int pid, ppid;
        sscanf(line, "%d %d", &pid, &ppid);

        if (pid == processID) {
            pclose(pipe);
            return ppid; // Return the parent process ID
        }
    }
    pclose(pipe);
    return -1; // If the process ID is not found, return -1
}

void searchSiblingProcesses(int processID) {
    // Get the parent process ID (ppid) of the specified processID
    int parentPID = getParentPID(processID);

    // If getParentPID returns -1, it means the specified processID doesn't exist
    if (parentPID == -1) {
        printf("Process with PID %d does not exist.\n", processID);
        return;
    }

    // Use searchDirectDescendants to find all direct descendants of the parent process
    searchDirectDescendants(parentPID);
}


int main(int argc, char *argv[]) {
    // Check if the user provided the parent and child PIDs
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
        if (strcmp(argv[3], "-rp") == 0){
            // Kill the processes found in the foundPIDs array
            for (int i = 0; i < numFound; ++i) {
                if (foundPIDs[i] == childPID)
                {
                    if (kill(foundPIDs[i], SIGKILL) == 0) {
                        printf("Process %d killed\n", foundPIDs[i]);
                    } else {
                        printf("Failed to kill process %d\n", foundPIDs[i]);
                    }
                }
            }
        }
        else if (strcmp(argv[3], "-pr") == 0){
            // Check if the parent process was created by the current user
            if (isProcessCreatedByMe(parentPID)){
                // Kill the parent process
                if (kill(parentPID, SIGKILL) == 0){
                    printf("Parent process %d killed\n", parentPID);
                }
                else{
                    printf("Failed to kill parent process %d\n", parentPID);
                }
            }
            else{
                printf("The parent process with PID %d was not created by you.\n", parentPID);
            }
            return 0;
        }
        else if (strcmp(argv[3], "-xn") == 0)
        {
            // Search for non-direct descendants and print their PIDs
            searchNonDirectDescendants(childPID);
            printf("Non-direct descendants of process %d:\n", childPID);
            for (int i = 0; i < numFound; ++i)
            {
                printf("%d\n", foundPIDs[i]);
            }
            return 0;
        }
        else if (strcmp(argv[3], "-xd") == 0)
        {
            // Search for non-direct descendants and print their PIDs
            searchDirectDescendants(childPID);
            printf("direct descendants of process %d:\n", childPID);
            for (int i = 0; i < numFound; ++i)
            {
                printf("%d\n", foundPIDs[i]);
            }
            return 0;
        }
        else if (strcmp(argv[3], "-xs") == 0)
        {
            // Search for non-direct descendants and print their PIDs
            searchSiblingProcesses(childPID);
            printf("siblings of process %d:\n", childPID);
            for (int i = 1; i < numFound; ++i)
            {
                printf("%d\n", foundPIDs[i]);
            }
            return 0;
        }
    }
    printf("Usage: %s <childPID> <parentPID>\n", argv[0]);
    return 0;
}
