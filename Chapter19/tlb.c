#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <pages> <trials>\n", argv[0]);
        return 1;
    }

    int pages = atoi(argv[1]);
    int trials = atoi(argv[2]);

    if (pages <= 0 || trials <= 0) {
        printf("Pages and trials must be positive.\n");
        return 1;
    }

    int page_size = getpagesize();

    // Allocate memory
    int *array = calloc((size_t)pages * page_size / sizeof(int),
                        sizeof(int));

    if (array == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Access each page once before timing
    for (int i = 0; i < pages; i++) {
        array[i * page_size / sizeof(int)] = 1;
    }

    struct timeval start, end;
    volatile int sum = 0;

    gettimeofday(&start, NULL);

    // Repeatedly access each page
    for (int t = 0; t < trials; t++) {
        for (int i = 0; i < pages; i++) {
            sum += array[i * page_size / sizeof(int)];
        }
    }

    gettimeofday(&end, NULL);

    double elapsed =
        (end.tv_sec - start.tv_sec) * 1000000.0 +
        (end.tv_usec - start.tv_usec);

    double avg_ns = elapsed * 1000.0 / ((double)pages * trials);

    printf("Pages: %d\n", pages);
    printf("Trials: %d\n", trials);
    printf("Page size: %d bytes\n", page_size);
    printf("Average access time: %.2f ns\n", avg_ns);

    free(array);

    return 0;
}