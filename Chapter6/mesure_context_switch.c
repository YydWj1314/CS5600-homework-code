#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/wait.h>

#define ITERATIONS 100000

double get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

void bind_to_cpu0() {
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(0, &set);

    if (sched_setaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
        exit(1);
    }
}

int main() {
    int pipe1[2];
    int pipe2[2];

    char byte = 'x';

    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(1);
    }

    if (pipe(pipe2) == -1) {
        perror("pipe2");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    /*
     * Child process
     */
    if (pid == 0) {

        bind_to_cpu0();

        /*
         * Child:
         *
         * read from pipe1
         * write to pipe2
         */

        close(pipe1[1]);
        close(pipe2[0]);

        for (int i = 0; i < ITERATIONS; i++) {

            if (read(pipe1[0], &byte, 1) != 1) {
                perror("child read");
                exit(1);
            }

            if (write(pipe2[1], &byte, 1) != 1) {
                perror("child write");
                exit(1);
            }
        }

        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    }

    /*
     * Parent process
     */
    else {

        bind_to_cpu0();

        /*
         * Parent:
         *
         * write to pipe1
         * read from pipe2
         */

        close(pipe1[0]);
        close(pipe2[1]);

        double start = get_time_us();

        for (int i = 0; i < ITERATIONS; i++) {

            if (write(pipe1[1], &byte, 1) != 1) {
                perror("parent write");
                exit(1);
            }

            if (read(pipe2[0], &byte, 1) != 1) {
                perror("parent read");
                exit(1);
            }
        }

        double end = get_time_us();

        wait(NULL);

        close(pipe1[1]);
        close(pipe2[0]);

        double total_time = end - start;

        /*
         * Each round-trip causes approximately
         * two context switches:
         *
         * parent -> child
         * child  -> parent
         */

        double context_switch_time =
            total_time / (ITERATIONS * 2.0);

        printf("Iterations: %d\n", ITERATIONS);
        printf("Total time: %.2f us\n", total_time);

        printf(
            "Average round-trip time: %.6f us\n",
            total_time / ITERATIONS
        );

        printf(
            "Estimated context switch cost: %.6f us\n",
            context_switch_time
        );

        printf(
            "Estimated context switch cost: %.2f ns\n",
            context_switch_time * 1000
        );
    }

    return 0;
}