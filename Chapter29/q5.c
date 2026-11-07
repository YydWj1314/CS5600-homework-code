#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

enum { BUCKETS = 64, MAX_THREADS = 8, UPDATES = 200000, TRIALS = 3 };

static long buckets[BUCKETS];
static pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    uint32_t seed = *(const uint32_t *)arg;

    for (int i = 0; i < UPDATES; i++) {
        seed = seed * UINT32_C(1664525) + UINT32_C(1013904223);
        unsigned bucket = (seed >> 16) % BUCKETS;

        /* One mutex protects updates to all 64 buckets. */
        pthread_mutex_lock(&table_lock);
        buckets[bucket]++;
        pthread_mutex_unlock(&table_lock);
    }
    return NULL;
}

static void check_thread_call(int error, const char *operation)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        exit(EXIT_FAILURE);
    }
}

static long long now_us(void)
{
    struct timeval time;
    if (gettimeofday(&time, NULL) != 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }
    return time.tv_sec * 1000000LL + time.tv_usec;
}

int main(void)
{
    printf("%7s %12s %15s\n", "Threads", "Updates", "Median (ms)");

    for (int n = 1; n <= MAX_THREADS; n *= 2) {
        double times[TRIALS];

        for (int trial = 0; trial < TRIALS; trial++) {
            pthread_t threads[MAX_THREADS];
            uint32_t seeds[MAX_THREADS];
            memset(buckets, 0, sizeof buckets);

            /* Include thread creation and joining in the elapsed time. */
            long long start = now_us();
            for (int i = 0; i < n; i++) {
                seeds[i] = (uint32_t)i + 1;
                check_thread_call(
                    pthread_create(&threads[i], NULL, worker, &seeds[i]),
                    "pthread_create");
            }
            for (int i = 0; i < n; i++) {
                check_thread_call(pthread_join(threads[i], NULL),
                                  "pthread_join");
            }
            times[trial] = (now_us() - start) / 1000.0;

            /* Verify that no updates were lost. */
            long total = 0;
            for (int bucket = 0; bucket < BUCKETS; bucket++) {
                total += buckets[bucket];
            }
            if (total != (long)n * UPDATES) {
                fprintf(stderr, "Incorrect update count: %ld\n", total);
                return EXIT_FAILURE;
            }
        }

        /* Sort the three timings and report the middle one. */
        for (int i = 1; i < TRIALS; i++) {
            for (int j = i; j > 0 && times[j] < times[j - 1]; j--) {
                double temp = times[j];
                times[j] = times[j - 1];
                times[j - 1] = temp;
            }
        }

        printf("%7d %12ld %15.3f\n", n, (long)n * UPDATES, times[1]);
    }

    pthread_mutex_destroy(&table_lock);
    return EXIT_SUCCESS;
}