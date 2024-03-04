#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

// Global array to store found processes
int branch_processes[100];
int branch_processes_zombie[100];
// Counter for total process found under a root_process
int total_process_found = 0;
int total_process_found_zombie = 0;


// Checking if a process was created by the logged-in user or not
// returns true/false
bool isProcessCreatedByMe(pid_t provided_UID) {
    // Retrieving the UID of the running process
    uid_t running_process_uid = getuid();

    // For storing the UID of the provided process
    uid_t provided_process_uid = 0;
    FILE *proc_file;
    char proc_file_name[100];
    char proc_file_line[100];
    // Storing the proc string in proc_file_name
    sprintf(proc_file_name, "/proc/%d/status", provided_UID);
    // Reading the proc file
    proc_file = fopen(proc_file_name, "r");
    if (proc_file == NULL) {
        perror("Failed opening proc file for provided process.");
        return false;
    }
    // Reading all the line until it finds the UID field
    while (fgets(proc_file_line, 100, proc_file) != NULL) {
        // Comparing first 4 character to confirm UID field
        if (strncmp(proc_file_line, "Uid:", 4) == 0) {
            // Reading the UID from the line and storing it to provided_process_uid
            sscanf(proc_file_line, "Uid: %d", &provided_process_uid);
            break;
        }
    }
    fclose(proc_file);

    // Returning true or false based on whether the UID of
    // provided_process_uid matches running_process_uid or not
    return (provided_process_uid == running_process_uid);
}


// Function to recursively search for the parent of the child process
// until the root_process is found. The whole branch from root_process to
// process_id is then stored in an array
// returns true/false
bool searchChildProcess(int process_id, int root_process) {
    // Reading the command to get all the processes
    FILE *ps_cmd_output = popen("ps -o pid,ppid -ax", "r");
    if (!ps_cmd_output) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output)) {
        // For holding the PID and PPID from the ps_cmd_line
        int pid, ppid;
        // Fetching pid and ppid to store them
        sscanf(ps_cmd_line, "%d %d", &pid, &ppid);
        // Check if the current process is the child process passed as a parameter
        if (pid == process_id) {
            // Check if the root_process provided as a param is the parent
            // of the current process_id
            if (ppid == root_process) {
                // If the root_process matches the ppid then this is indeed the parent
                // of the provided process_id. So, it will be now stored in the array
                branch_processes[total_process_found++] = pid;
                pclose(ps_cmd_output);
                return true;
            }
                // If the root_process is not the parent of the provided process_id,
                // we will recursively keep searching
                // for the ppid of the parent process of the child process
            else {
                // Recursive parent search call
                bool found = searchChildProcess(ppid, root_process);
                // Finally if the parent of any process is same as the root_process
                // then it means the initial process_id is actually a member of the root_process
                if (found) {
                    // Store the child PID in the global array
                    branch_processes[total_process_found++] = pid;
                    pclose(ps_cmd_output);
                    return true;
                }
            }
        }
    }
    // Close the ps_cmd_output
    pclose(ps_cmd_output);
    return false;
}


// Function to recursively search for non-direct descendants of provided
// process_id and storing then in branch_processes
// returns nothing but stores data in branch_processes array
void searchNonDirectDescendants(int process_id)
{
    total_process_found = 0;
    // Reading the command to get all the processes
    FILE *ps_cmd_output = popen("ps -o pid,ppid -ax", "r");
    if (!ps_cmd_output)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);

    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output))
    {
        // For holding the PID and PPID from the ps_cmd_line
        int pid, ppid;
        // Fetching pid and ppid to store them
        sscanf(ps_cmd_line, "%d %d", &pid, &ppid);
        // Check if the ppid of the current process is not same as process_id which
        // proves it is not a direct descendant and also making sure it is indeed a member
        // in the process_id's branch by checking searchChildProcess
        if (ppid != process_id && searchChildProcess(ppid, process_id))
        {
            // Storing in branch_processes if it is a non-direct descendant
            branch_processes[total_process_found++] = pid;
        }
    }
    // Close the ps_cmd_output
    pclose(ps_cmd_output);
}


// Function for searching all the direct descendants
// returns nothing but stores data in branch_processes array
void searchDirectDescendants(int process_id) {
    // Clear the branch_processes array before populating it with new direct descendants
    total_process_found = 0;

    // Reading the command to get all the processes
    FILE *ps_cmd_output = popen("ps -o pid,ppid -ax", "r");
    if (!ps_cmd_output) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output)) {
        // For holding the PID and PPID from the ps_cmd_line
        int pid, ppid;
        // Fetching pid and ppid to store them
        sscanf(ps_cmd_line, "%d %d", &pid, &ppid);

        // If the ppid of current process is same as the process_id
        // it means it is the direct child of process_id
        if (ppid == process_id) {
            // Storing in branch_processes if it is a direct descendant
            branch_processes[total_process_found++] = pid;
        }
    }
    // Close the ps_cmd_output
    pclose(ps_cmd_output);
}


// Function to get the parent process ID of process_id
// returns parent process id or -1
int getParentPID(int processID) {
    // Reading the command to get all the processes
    FILE *ps_cmd_output = popen("ps -o pid,ppid -ax", "r");
    if (!ps_cmd_output) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output)) {
        // For holding the PID and PPID from the ps_cmd_line
        int pid, ppid;
        // Fetching pid and ppid to store them
        sscanf(ps_cmd_line, "%d %d", &pid, &ppid);

        // if the current process is the process_id then returning the
        // parent process id of it
        if (pid == processID) {
            pclose(ps_cmd_output);
            return ppid;
        }
    }
    pclose(ps_cmd_output);
    return -1;
}


// Searches for the sibling processes of the provided process_id by searching
// all the child of the parent process of process_id
void searchSiblingProcesses(int process_id) {
    // Retrieving the parent process id of the process_id
    int parent_process_id = getParentPID(process_id);

    if (parent_process_id == -1) {
        printf("Process with PID %d does not exist.\n", process_id);
        return;
    }

    // Using searchDirectDescendants to find all direct descendants of the parent_process_id
    searchDirectDescendants(parent_process_id);
}


// Function for storing the paused processes to a file
void storePausedProcessToFile(pid_t pid) {
    // Open the file in append mode
    FILE *file = fopen("paused_processes.txt", "a");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d\n", pid);
    fclose(file);
}


// Function for continuing the paused processes
void continuePausedProcesses() {
    // Open the file for reading
    FILE *file = fopen("paused_processes.txt", "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Reading all the processes from the file to continue them
    char paused_process_ids[256];
    while (fgets(paused_process_ids, sizeof(paused_process_ids), file)) {
        pid_t pid = atoi(paused_process_ids);

        // Send SIGCONT signal to the process
        if (kill(pid, SIGCONT) == 0) {
            printf("Continued process %d\n", pid);
        } else {
            perror("Failed to continue process");
        }
    }
    fclose(file);

    // Clearing the file content
    file = fopen("paused_processes.txt", "w");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}


// Function for pausing a process
void pauseProcess(int process_id) {
    // Sending the SIGSTOP signal to pause the process
    if (kill(process_id, SIGSTOP) == 0) {
        printf("%d paused\n", process_id);
        // Storing the paused processes
        storePausedProcessToFile(process_id);
    } else {
        perror("Failed to pause process");
    }
}


// Function to recursively search for defunct processes under process_id
void searchDefunctProcesses(int process_id) {
    // Reading the command to get all the processes
    FILE *ps_cmd_output = popen("ps -o pid,state,ppid -ax", "r");
    if (!ps_cmd_output) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output)) {
        // Extracting the PID, state, and PPID from the ps_cmd_line
        int pid, ppid;
        char state;
        sscanf(ps_cmd_line, "%d %c %d", &pid, &state, &ppid);
        // Checking if the current process is a descendant of the specified parent process
        if (searchChildProcess(pid, process_id)) {
            // Checking if the process is a Zombie or not
            if (state == 'Z') {
                // Storing in branch_processes
                branch_processes_zombie[total_process_found_zombie++] = pid;
            }
        }
    }
    // Close the ps_cmd_output
    pclose(ps_cmd_output);
}


// Function for searching all the grandchild's
void searchGrandchildProcesses(int parentPID) {
    // Clear the branch_processes array before populating it with new grandchild processes
    total_process_found = 0;

    // Open the ps_cmd_output to read the output of the ps command
    FILE *ps_cmd_output = popen("ps -o pid,ppid -ax", "r");
    if (!ps_cmd_output) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[BUFFER_SIZE];
    // Reading the first line which contains the header
    // Ultimately skipping it
    fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output);
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, BUFFER_SIZE, ps_cmd_output)) {
        // Extracting the PID and PPID from the ps_cmd_line
        int pid, ppid;
        sscanf(ps_cmd_line, "%d %d", &pid, &ppid);

        // Get the parent's parent PID (grandparent)
        int grandparentPID = getParentPID(getParentPID(pid));

        // Check if the grandparent PID matches the specified parent process ID
        if (grandparentPID == parentPID) {
            // Store the PID of the grandchild process at the end of the branch_processes array
            branch_processes[total_process_found++] = pid;
        }
    }
    // Close the ps_cmd_output
    pclose(ps_cmd_output);
}


// Function for printing a process status whether it is a Zombie or not
void printProcessStatus(int processID) {
    char filename[100];
    // Storing filename to filename
    sprintf(filename, "/proc/%d/status", processID);

    // Open the file for reading
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char ps_cmd_line[256];
    bool found = false;
    // Finally reading all the lines generated by ps command
    while (fgets(ps_cmd_line, sizeof(ps_cmd_line), file)) {
        // Searching for the sub-string
        if (strstr(ps_cmd_line, "State:") != NULL) {
            // Extract the process state from the ps_cmd_line
            char state;
            // When foung the value of state is stored in state variable
            sscanf(ps_cmd_line, "State: %c", &state);

            if (state == 'Z') {
                printf("Process %d is defunct (Zombie)\n", processID);
            } else {
                printf("Process %d is not defunct\n", processID);
            }
            found = true;
            break;
        }
    }
    fclose(file);
    if (!found) {
        printf("Process with PID %d does not exist.\n", processID);
    }
}


int main(int argc, char *argv[]) {
    // checking for minimum argument number
    if (argc < 3)
    {
        printf("Please follow the provided format: %s <process_id> <root_process> <command>(optional)\n", argv[0]);
        return 0;
    }

    // Preparing process_id & root_process
    int process_id = atoi(argv[1]);
    int root_process = atoi(argv[2]);

    // Checks if the root_process is created by the logged-in user or not
    if (!isProcessCreatedByMe(root_process)){
        printf("Can not perform task on %d as it was not created by logged-in user.\n", root_process);
        exit(0);
    }
    // Checking whether process_id is under root_process or not
    if (!searchChildProcess(process_id, root_process)) {
        printf("Does not belong to the process tree \n");
        exit(0);
    }

    // For the default command
    if (argc == 3) {
        // As branch_processes already contains all the branch members from provided
        // root_process to process_id, printing the last two pid will be enough
        for (int i = total_process_found - 1; i >= total_process_found - 2 && i >= 0; --i) {
            printf("%d ", branch_processes[i]);
        }
        printf("\n");
        return 0;
    }
    // for all other 10 commands
    if (argc == 4) {
        // -rp process_id is killed if it belongs to the process tree rooted at root_process
        if (strcmp(argv[3], "-rp") == 0){
            // Searching for the process_id in total_process_found array
            // If it exists it is a member.
            for (int i = 0; i < total_process_found; ++i) {
                if (branch_processes[i] == process_id)
                {
                    // Killing the process
                    if (kill(branch_processes[i], SIGKILL) == 0) {
                        printf("%d killed\n", branch_processes[i]);
                    } else {
                        printf("Failed to kill %d\n", branch_processes[i]);
                    }
                }
            }
            return 0;
        }
            // -pr the root_process is killed (if it is valid)
        else if (strcmp(argv[3], "-pr") == 0){
            // Checking if the root_process was created by the current user
            // using the same isProcessCreatedByMe method
            if (isProcessCreatedByMe(root_process)){
                // Killing the process
                if (kill(root_process, SIGKILL) == 0){
                    printf("Parent process %d killed\n", root_process);
                }
                else{
                    printf("Failed killing the parent process %d\n", root_process);
                }
            }
            else{
                printf("The parent process with PID %d was not created by logged-in user.\n", root_process);
            }
            return 0;
        }
            // -xn lists the PIDs of all the non-direct descendants of process_id
        else if (strcmp(argv[3], "-xn") == 0){
            // Searching for the non-direct descendants
            searchNonDirectDescendants(process_id);
            if (total_process_found == 0)
            {
                printf("No non-direct descendants\n");
                return 0;
            }
            for (int i = 0; i < total_process_found; ++i)
            {
                printf("%d\n", branch_processes[i]);
            }
            return 0;
        }
            // -xd lists the PIDs of all the immediate descendants of process_id
        else if (strcmp(argv[3], "-xd") == 0){
            // Searching for the direct descendants
            searchDirectDescendants(process_id);
            if (total_process_found == 0)
            {
                printf("No direct descendants\n");
                return 0;
            }
            for (int i = 0; i < total_process_found; ++i)
            {
                printf("%d\n", branch_processes[i]);
            }
            return 0;
        }
            //  -xs lists the PIDs of all the sibling processes of process_id
        else if (strcmp(argv[3], "-xs") == 0){
            // Searching for sibling processes
            searchSiblingProcesses(process_id);
            if (total_process_found == 0)
            {
                printf("No sibling/s \n");
                return 0;
            }
            for (int i = 1; i < total_process_found; ++i)
            {
                printf("%d\n", branch_processes[i]);
            }
            return 0;
        }
            // -xt process_id is paused with SIGSTOP
        else if (strcmp(argv[3], "-xt") == 0){
            pauseProcess(process_id);
            return 0;
        }
            // -xc SIGCONT is sent to all processes that have been paused earlier
        else if (strcmp(argv[3], "-xc") == 0){
            continuePausedProcesses();
            return 0;
        }
            // - xz Lists the PIDs of all descendents of process_id that are defunct
        else if (strcmp(argv[3], "-xz") == 0){
            // Searching <defunct> processes
            searchDefunctProcesses(process_id);
            if (total_process_found_zombie == 0)
            {
                printf("No descendant zombie process/es\n");
                return 0;
            }
            // Printing the PIDs of defunct processes
            for (int i = 0; i < total_process_found_zombie; ++i) {
                printf("%d\n", branch_processes_zombie[i]);
            }
            return 0;
        }
            // - xg lists the PIDs of all the grandchildren of process_id
        else if (strcmp(argv[3], "-xg") == 0){
            searchGrandchildProcesses(process_id);
            if (total_process_found == 0)
            {
                printf("No grandchildren\n");
                return 0;
            }
            // Print the PIDs of defunct processes
            for (int i = 0; i < total_process_found; i++) {
                printf("%d\n", branch_processes[i]);
            }
            return 0;
        }
            // - zs prints the status of process_id (Defunct/ Not Defunct)
        else if (strcmp(argv[3], "-zs") == 0){
            printProcessStatus(process_id);
            return 0;
        }
    }
    printf("Please follow the provided format: %s <process_id> <root_process> <command>(optional)\n", argv[0]);
    return 0;
}