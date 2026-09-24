#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define ITERATIONS 1000000

double get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

int main() {
    int fd = open("/dev/null", O_RDONLY);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    double start = get_time_us();

    for (long i = 0; i < ITERATIONS; i++) {
        read(fd, NULL, 0);
    }

    double end = get_time_us();

    close(fd);

    double total_time = end - start;
    double avg_time = total_time / ITERATIONS;

    printf("Iterations: %d\n", ITERATIONS);
    printf("Total time: %.2f us\n", total_time);
    printf("Average system call cost: %.6f us\n", avg_time);
    printf("Average system call cost: %.2f ns\n", avg_time * 1000);

    return 0;
}