#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common_threads.h"

typedef struct __barrier_t {
    sem_t lock;       /* Protects the arrival counter. */
    sem_t gate;       /* Blocks threads until everyone arrives. */
    int arrived;
    int num_threads;
} barrier_t;

barrier_t b;

static void check_sem(int result, const char *operation)
{
    if (result == -1) {
        perror(operation);
        exit(EXIT_FAILURE);
    }
}

static void wait_sem(sem_t *s)
{
    int result;

    do {
        result = sem_wait(s);
    } while (result == -1 && errno == EINTR);

    check_sem(result, "sem_wait");
}

void barrier_init(barrier_t *b, int num_threads)
{
    b->arrived = 0;
    b->num_threads = num_threads;

    check_sem(sem_init(&b->lock, 0, 1), "sem_init lock");
    check_sem(sem_init(&b->gate, 0, 0), "sem_init gate");
}

void barrier(barrier_t *b)
{
    wait_sem(&b->lock);
    b->arrived++;

    if (b->arrived == b->num_threads) {
        /* The last arrival posts one permit per thread, including itself. */
        for (int i = 0; i < b->num_threads; i++) {
            check_sem(sem_post(&b->gate), "sem_post gate");
        }
    }

    check_sem(sem_post(&b->lock), "sem_post lock");
    wait_sem(&b->gate);
}

//
// XXX: don't change below here (just run it!)
//
typedef struct __tinfo_t {
    int thread_id;
} tinfo_t;

void *child(void *arg) {
    tinfo_t *t = (tinfo_t *) arg;
    printf("child %d: before\n", t->thread_id);
    barrier(&b);
    printf("child %d: after\n", t->thread_id);
    return NULL;
}

// run with a single argument indicating the number of
// threads you wish to create (1 or more)
int main(int argc, char *argv[]) {
    assert(argc == 2);
    int num_threads = atoi(argv[1]);
    assert(num_threads > 0);

    pthread_t p[num_threads];
    tinfo_t t[num_threads];

    printf("parent: begin\n");
    barrier_init(&b, num_threads);

    int i;
    for (i = 0; i < num_threads; i++) {
        t[i].thread_id = i;
        Pthread_create(&p[i], NULL, child, &t[i]);
    }

    for (i = 0; i < num_threads; i++)
        Pthread_join(p[i], NULL);

    printf("parent: end\n");
    return 0;
}