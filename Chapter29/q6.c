#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

enum {
    BUCKETS = 64,
    MAX_THREADS = 8,
    UPDATES = 200000,
    TRIALS = 3
};

typedef enum {
    TABLE_LOCK,
    BUCKET_LOCKS
} LockStrategy;

typedef struct {
    uint32_t seed;
    LockStrategy strategy;
} WorkerArgs;

static long buckets[BUCKETS];
static pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t bucket_locks[BUCKETS];

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

static void *worker(void *arg)
{
    const WorkerArgs *args = arg;
    uint32_t seed = args->seed;

    for (int i = 0; i < UPDATES; i++) {
        seed = seed * UINT32_C(1664525) + UINT32_C(1013904223);
        unsigned bucket = (seed >> 16) % BUCKETS;

        /* Both strategies update the same bucket. Only the lock differs. */
        pthread_mutex_t *lock = args->strategy == TABLE_LOCK
                             ? &table_lock
                             : &bucket_locks[bucket];

        pthread_mutex_lock(lock);
        buckets[bucket]++;
        pthread_mutex_unlock(lock);
    }

    return NULL;
}

static double run_trial(int thread_count, LockStrategy strategy)
{
    pthread_t threads[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    memset(buckets, 0, sizeof buckets);

    /* Reuse the same key sequence for each thread in both strategies. */
    for (int i = 0; i < thread_count; i++) {
        args[i].seed = (uint32_t)i + 1;
        args[i].strategy = strategy;
    }

    /* Time thread creation, updates, and joining. */
    long long start = now_us();

    for (int i = 0; i < thread_count; i++) {
        check_thread_call(
            pthread_create(&threads[i], NULL, worker, &args[i]),
            "pthread_create");
    }

    for (int i = 0; i < thread_count; i++) {
        check_thread_call(pthread_join(threads[i], NULL), "pthread_join");
    }

    long long elapsed = now_us() - start;

    /* Verify the total after all workers have finished. */
    long total = 0;
    for (int i = 0; i < BUCKETS; i++) {
        total += buckets[i];
    }

    long expected = (long)thread_count * UPDATES;
    if (total != expected) {
        fprintf(stderr, "Expected %ld updates, got %ld\n", expected, total);
        exit(EXIT_FAILURE);
    }

    if (elapsed <= 0) {
        fprintf(stderr, "Invalid elapsed time. Rerun the benchmark.\n");
        exit(EXIT_FAILURE);
    }

    return elapsed / 1000.0;
}

static double median_time(int thread_count, LockStrategy strategy)
{
    double times[TRIALS];

    for (int i = 0; i < TRIALS; i++) {
        times[i] = run_trial(thread_count, strategy);
    }

    for (int i = 1; i < TRIALS; i++) {
        for (int j = i; j > 0 && times[j] < times[j - 1]; j--) {
            double temp = times[j];
            times[j] = times[j - 1];
            times[j - 1] = temp;
        }
    }

    return times[TRIALS / 2];
}

int main(void)
{
    for (int i = 0; i < BUCKETS; i++) {
        check_thread_call(pthread_mutex_init(&bucket_locks[i], NULL),
                          "pthread_mutex_init");
    }

    printf("%7s %12s %16s %17s %9s\n",
           "Threads", "Updates", "Table lock (ms)",
           "Bucket locks (ms)", "Speedup");

    for (int n = 1; n <= MAX_THREADS; n *= 2) {
        double table_ms = median_time(n, TABLE_LOCK);
        double bucket_ms = median_time(n, BUCKET_LOCKS);

        printf("%7d %12ld %16.3f %17.3f %8.2fx\n",
               n, (long)n * UPDATES, table_ms, bucket_ms,
               table_ms / bucket_ms);
    }

    for (int i = 0; i < BUCKETS; i++) {
        check_thread_call(pthread_mutex_destroy(&bucket_locks[i]),
                          "pthread_mutex_destroy");
    }

    check_thread_call(pthread_mutex_destroy(&table_lock),
                      "pthread_mutex_destroy");

    return EXIT_SUCCESS;
}