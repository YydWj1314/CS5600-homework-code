#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process (PID=%d)\n", getpid());

        /************* Uncomment ONE of the following *************/

        // 1. execl()
        execl("/bin/ls", "ls", "-l", NULL);

        // 2. execv()
        // char *args[] = {"ls", "-l", NULL};
        // execv("/bin/ls", args);

        // 3. execlp()
        // execlp("ls", "ls", "-l", NULL);

        // 4. execvp()
        // char *args[] = {"ls", "-l", NULL};
        // execvp("ls", args);

        // 5. execle()
        // char *env[] = {NULL};
        // execle("/bin/ls", "ls", "-l", NULL, env);

        /*******************************************************/

        // Only executes if exec() fails
        perror("exec failed");
    }
    else {
        printf("Parent process (PID=%d)\n", getpid());
    }

    return 0;
}