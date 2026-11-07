#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

enum { MAX_THREADS = 8, INCREMENTS = 200000 };

static long counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *increment_counter(void *arg)
{
    (void)arg;

    for (int i = 0; i < INCREMENTS; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(void)
{
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus > 0) {
        printf("Online logical CPUs: %ld\n", cpus);
    }

    printf("%7s %12s %12s %16s\n",
           "Threads", "Final count", "Time (ms)", "Increments/s");

    for (int n = 1; n <= MAX_THREADS; n *= 2) {
        pthread_t threads[MAX_THREADS];
        struct timeval start, end;
        counter = 0;

        if (gettimeofday(&start, NULL) == -1) {
            perror("gettimeofday");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < n; i++) {
            int error = pthread_create(&threads[i], NULL, increment_counter, NULL);
            if (error != 0) {
                fprintf(stderr, "pthread_create: %s\n", strerror(error));
                return EXIT_FAILURE;
            }
        }

        for (int i = 0; i < n; i++) {
            int error = pthread_join(threads[i], NULL);
            if (error != 0) {
                fprintf(stderr, "pthread_join: %s\n", strerror(error));
                return EXIT_FAILURE;
            }
        }

        if (gettimeofday(&end, NULL) == -1) {
            perror("gettimeofday");
            return EXIT_FAILURE;
        }

        long expected = (long)n * INCREMENTS;
        if (counter != expected) {
            fprintf(stderr, "Expected %ld, got %ld\n", expected, counter);
            return EXIT_FAILURE;
        }

        double seconds = (end.tv_sec - start.tv_sec)
                       + (end.tv_usec - start.tv_usec) / 1000000.0;
        if (seconds <= 0.0) {
            fprintf(stderr, "Invalid elapsed time. Rerun the benchmark.\n");
            return EXIT_FAILURE;
        }

        printf("%7d %12ld %12.3f %16.0f\n",
               n, counter, seconds * 1000.0, counter / seconds);
    }

    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}