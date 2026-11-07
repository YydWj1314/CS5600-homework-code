#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

enum { NODES = 128, MAX_THREADS = 8, LOOKUPS = 10000 };

typedef struct Node {
    int key;
    struct Node *next;
    pthread_mutex_t lock;
} Node;

typedef struct {
    unsigned int seed;
    int coupled;
    int found;
} WorkerArgs;

static Node nodes[NODES];
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

static int lookup_global(int key)
{
    pthread_mutex_lock(&list_lock);

    Node *current = &nodes[0];
    while (current != NULL && current->key != key) {
        current = current->next;
    }

    int found = current != NULL;
    pthread_mutex_unlock(&list_lock);
    return found;
}

static int lookup_coupled(int key)
{
    Node *current = &nodes[0];
    pthread_mutex_lock(&current->lock);

    while (current->key != key && current->next != NULL) {
        Node *next = current->next;

        /* Lock the next node before releasing the current node. */
        pthread_mutex_lock(&next->lock);
        pthread_mutex_unlock(&current->lock);
        current = next;
    }

    int found = current->key == key;
    pthread_mutex_unlock(&current->lock);
    return found;
}

static void *worker(void *arg)
{
    WorkerArgs *args = arg;
    unsigned int seed = args->seed;
    int (*lookup)(int) = args->coupled ? lookup_coupled : lookup_global;

    /* Reuse the same key sequence for each locking strategy. */
    for (int i = 0; i < LOOKUPS; i++) {
        seed = seed * 1664525u + 1013904223u;
        int key = (seed >> 16) % NODES;
        args->found += lookup(key);
    }

    return NULL;
}

static double benchmark(int thread_count, int coupled)
{
    pthread_t threads[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];
    struct timeval start, end;

    for (int i = 0; i < thread_count; i++) {
        args[i] = (WorkerArgs){.seed = (unsigned int)i + 1,
                              .coupled = coupled, .found = 0};
    }

    /* Measure wall-clock time from thread creation through the last join. */
    if (gettimeofday(&start, NULL) != 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "pthread_join failed\n");
            exit(EXIT_FAILURE);
        }
    }

    if (gettimeofday(&end, NULL) != 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }

    /* Every generated key exists in the prebuilt list. */
    for (int i = 0; i < thread_count; i++) {
        if (args[i].found != LOOKUPS) {
            fprintf(stderr, "Thread %d missed an existing key\n", i);
            exit(EXIT_FAILURE);
        }
    }

    double seconds = (end.tv_sec - start.tv_sec)
                   + (end.tv_usec - start.tv_usec) / 1000000.0;
    if (seconds <= 0.0) {
        fprintf(stderr, "Invalid elapsed time. Rerun the benchmark.\n");
        exit(EXIT_FAILURE);
    }
    return seconds;
}

int main(void)
{
    /* Build the list before starting any benchmark threads. */
    for (int i = 0; i < NODES; i++) {
        nodes[i].key = i;
        nodes[i].next = (i + 1 < NODES) ? &nodes[i + 1] : NULL;

        if (pthread_mutex_init(&nodes[i].lock, NULL) != 0) {
            fprintf(stderr, "pthread_mutex_init failed\n");
            return EXIT_FAILURE;
        }
    }

    printf("%-17s %7s %12s %15s\n",
           "Locking", "Threads", "Time (ms)", "Lookups/s");

    for (int n = 1; n <= MAX_THREADS; n *= 2) {
        for (int coupled = 0; coupled <= 1; coupled++) {
            double seconds = benchmark(n, coupled);
            printf("%-17s %7d %12.3f %15.0f\n",
                   coupled ? "Hand-over-hand" : "Global lock",
                   n, seconds * 1000.0, (n * LOOKUPS) / seconds);
        }
    }

    for (int i = 0; i < NODES; i++) {
        pthread_mutex_destroy(&nodes[i].lock);
    }
    pthread_mutex_destroy(&list_lock);
    return EXIT_SUCCESS;
}