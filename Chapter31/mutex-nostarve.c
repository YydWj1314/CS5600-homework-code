#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_threads.h"

/*
 * Morris's mutex serves threads in batches. New arrivals wait
 * until the current batch finishes, so each waiting thread
 * eventually gets a turn if threads are scheduled and release the lock.
 *
 * Test: all four threads acquire the lock 1,000 times each,
 * and the final counter is 4,000.
 */

typedef struct __ns_mutex_t {
    sem_t guard;       /* Protects room1. */
    sem_t first_gate;  /* Moves threads from room 1 to room 2. */
    sem_t second_gate; /* Admits one thread to the critical section. */
    int room1;
    int room2;
} ns_mutex_t;

static void check_sem(int result, const char *operation) {
    if (result == -1) {
        perror(operation);
        exit(EXIT_FAILURE);
    }
}

static void wait_sem(sem_t *sem) {
    int result;

    do {
        result = sem_wait(sem);
    } while (result == -1 && errno == EINTR);

    check_sem(result, "sem_wait");
}

static void post_sem(sem_t *sem) {
    check_sem(sem_post(sem), "sem_post");
}

void ns_mutex_init(ns_mutex_t *m) {
    m->room1 = 0;
    m->room2 = 0;

    check_sem(sem_init(&m->guard, 0, 1), "sem_init guard");
    check_sem(sem_init(&m->first_gate, 0, 1), "sem_init first_gate");
    check_sem(sem_init(&m->second_gate, 0, 0), "sem_init second_gate");
}

void ns_mutex_acquire(ns_mutex_t *m) {
    wait_sem(&m->guard);
    m->room1++;
    post_sem(&m->guard);

    wait_sem(&m->first_gate);
    m->room2++;

    wait_sem(&m->guard);
    m->room1--;

    if (m->room1 == 0) {
        /* The last thread entering room 2 opens the second gate. */
        post_sem(&m->guard);
        post_sem(&m->second_gate);
    } else {
        post_sem(&m->guard);
        post_sem(&m->first_gate);
    }

    wait_sem(&m->second_gate);
    m->room2--;
}

void ns_mutex_release(ns_mutex_t *m) {
    if (m->room2 == 0) {
        /* The current batch is done. Let the next batch enter room 2. */
        post_sem(&m->first_gate);
    } else {
        /* Give the next thread in room 2 a turn. */
        post_sem(&m->second_gate);
    }
}

enum { THREADS = 4, LOOPS = 1000 };

static ns_mutex_t mutex;
static int counter;
static int acquisitions[THREADS];

void *worker(void *arg) {
    int id = *(const int *)arg;

    for (int i = 0; i < LOOPS; i++) {
        ns_mutex_acquire(&mutex);
        counter++;
        acquisitions[id]++;
        ns_mutex_release(&mutex);
    }

    return NULL;
}

int main(void) {
    pthread_t threads[THREADS];
    int ids[THREADS];

    printf("parent: begin\n");
    ns_mutex_init(&mutex);

    for (int i = 0; i < THREADS; i++) {
        ids[i] = i;
        Pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < THREADS; i++) {
        Pthread_join(threads[i], NULL);
    }

    int success = counter == THREADS * LOOPS;

    for (int i = 0; i < THREADS; i++) {
        printf("thread %d: acquired %d times\n", i, acquisitions[i]);
        if (acquisitions[i] != LOOPS) {
            success = 0;
        }
    }

    printf("counter = %d (expected %d)\n", counter, THREADS * LOOPS);

    check_sem(sem_destroy(&mutex.second_gate), "sem_destroy second_gate");
    check_sem(sem_destroy(&mutex.first_gate), "sem_destroy first_gate");
    check_sem(sem_destroy(&mutex.guard), "sem_destroy guard");

    printf("parent: end\n");
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}