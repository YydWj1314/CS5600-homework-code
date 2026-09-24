#include <stdio.h>
#include <unistd.h>

int main() {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();

    if (pid == 0) {
        // child
        close(pipefd[0]);

        printf("hello\n");
        fflush(stdout);

        write(pipefd[1], "x", 1);
        close(pipefd[1]);
    } else {
        // parent
        close(pipefd[1]);

        char buf;
        read(pipefd[0], &buf, 1);

        printf("goodbye\n");
        close(pipefd[0]);
    }

    return 0;
}