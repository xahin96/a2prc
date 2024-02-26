#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

long isValidNumber(char* str) {
    char *endptr;
    errno = 0;
    long result = strtol(str, &endptr, 10);
    if (endptr == str)
    {
        printf("must be a valid number\n");
    }
    if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE)
    {
        printf("must be a valid number\n");
    }

    return result;
}

int main(int argc, char *argv[]) {
    printf("Main Process ID: %d\n", getpid());

    int sleepTime = (int) isValidNumber(argv[1]);

    fork();
    fork();
    fork();
    fork();

    printf("Child Process ID: %d, Parent Process ID: %d\n", getpid(), getppid());
    sleep(sleepTime); // Sleep for N minutes

    return 0;
}