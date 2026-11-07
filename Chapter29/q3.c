#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

enum { MAX_THREADS = 8, INCREMENTS = 200000 };

typedef struct {
    int threshold;
    long remainder;
} WorkerArgs;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static long global_count;

static void *increment_counter(void *arg)
{
    WorkerArgs *worker = arg;
    long local_count = 0;

    for (int i = 0; i < INCREMENTS; i++) {
        local_count++;

        /* Merge a batch only when the local count reaches the threshold. */
        if (local_count == worker->threshold) {
            pthread_mutex_lock(&lock);
            global_count += local_count;
            pthread_mutex_unlock(&lock);
            local_count = 0;
        }
    }

    /* The main thread reads this remainder after pthread_join(). */
    worker->remainder = local_count;
    return NULL;
}

int main(void)
{
    const int thresholds[] = {1, 32, 1024};

    printf("%9s %7s %12s %12s %12s %16s\n",
           "Threshold", "Threads", "Global", "Exact",
           "Time (ms)", "Increments/s");

    for (size_t s = 0; s < sizeof thresholds / sizeof thresholds[0]; s++) {
        for (int n = 1; n <= MAX_THREADS; n *= 2) {
            pthread_t threads[MAX_THREADS];
            WorkerArgs args[MAX_THREADS];
            struct timeval start, end;

            global_count = 0;

            /* Time the workload, including thread creation and joining. */
            if (gettimeofday(&start, NULL) == -1) {
                perror("gettimeofday");
                return EXIT_FAILURE;
            }

            for (int i = 0; i < n; i++) {
                args[i].threshold = thresholds[s];
                int error = pthread_create(&threads[i], NULL,
                                           increment_counter, &args[i]);
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

            /* Recover the exact total by adding the unmerged remainders. */
            long exact_count = global_count;
            for (int i = 0; i < n; i++) {
                exact_count += args[i].remainder;
            }

            if (exact_count != (long)n * INCREMENTS) {
                fprintf(stderr, "Incorrect final count: %ld\n", exact_count);
                return EXIT_FAILURE;
            }

            double seconds = (end.tv_sec - start.tv_sec)
                           + (end.tv_usec - start.tv_usec) / 1000000.0;
            if (seconds <= 0.0) {
                fprintf(stderr, "Invalid elapsed time. Rerun the benchmark.\n");
                return EXIT_FAILURE;
            }

            printf("%9d %7d %12ld %12ld %12.3f %16.0f\n",
                   thresholds[s], n, global_count, exact_count,
                   seconds * 1000.0, exact_count / seconds);
        }
    }

    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}