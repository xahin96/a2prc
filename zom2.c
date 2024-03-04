#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;
    int i;

    // First level
    printf("Level 1: Parent PID: %d\n", getpid());
    for (i = 0; i < 2; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Second level
            printf("Level 2: Parent PID: %d, Child PID: %d\n", getppid(), getpid());
            for (i = 0; i < 2; i++) {
                pid = fork();
                if (pid < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
                    // Third level
                    printf("Level 3: Parent PID: %d, Child PID: %d\n", getppid(), getpid());
                    sleep(300); // Sleep for 5 minutes
                    exit(EXIT_SUCCESS);
                }
            }
            sleep(300); // Sleep for 5 minutes
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for child processes to finish
    for (i = 0; i < 3; i++) {
        wait(NULL);
    }

    return 0;
}
