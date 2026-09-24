#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int x = 100;

    pid_t pid = fork();

    if (pid == 0) {
        // child
        printf("Child: x = %d\n", x);
        x = 200;
        printf("Child after change: x = %d\n", x);
    } else {
        // parent
        printf("Parent: x = %d\n", x);
        x = 300;
        printf("Parent after change: x = %d\n", x);
    }

    return 0;
}