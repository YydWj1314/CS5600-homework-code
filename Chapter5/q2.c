#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    int fd = open("output.txt",
                  O_CREAT | O_WRONLY | O_TRUNC,
                  0644);

    pid_t pid = fork();

if (pid == 0) {
    for (int i = 0; i < 20; i++)
        write(fd, "Child\n", 6);

} else {
    for (int i = 0; i < 20; i++)
        write(fd, "Parent\n", 7);

}

    close(fd);

    return 0;
}