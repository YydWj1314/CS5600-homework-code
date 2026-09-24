#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child (PID=%d) is running\n", getpid());

        int status;
        pid_t ret = wait(&status);

        printf("Child: wait() returned %d\n", ret);

        sleep(2);

        printf("Child ends\n");
    } else {
        printf("Parent waiting...\n");

        int status;
        pid_t child_pid = wait(&status);

        printf("Parent: wait() returned %d\n", child_pid);
        printf("Parent ends\n");
    }

    return 0;
}