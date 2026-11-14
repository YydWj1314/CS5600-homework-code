#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
    sem_t reader_guard;  /* Protects the reader count. */
    sem_t resource;     /* Held by one writer or a group of readers. */
    int readers;
} rwlock_t;

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

void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    check_sem(sem_init(&rw->reader_guard, 0, 1), "sem_init reader_guard");
    check_sem(sem_init(&rw->resource, 0, 1), "sem_init resource");
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    wait_sem(&rw->reader_guard);

    rw->readers++;
    if (rw->readers == 1) {
        /* The first reader reserves the resource for all readers. */
        wait_sem(&rw->resource);
    }

    check_sem(sem_post(&rw->reader_guard), "sem_post reader_guard");
}

void rwlock_release_readlock(rwlock_t *rw) {
    wait_sem(&rw->reader_guard);

    rw->readers--;
    if (rw->readers == 0) {
        /* The last reader releases the shared resource. */
        check_sem(sem_post(&rw->resource), "sem_post resource");
    }

    check_sem(sem_post(&rw->reader_guard), "sem_post reader_guard");
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    wait_sem(&rw->resource);
}

void rwlock_release_writelock(rwlock_t *rw) {
    check_sem(sem_post(&rw->resource), "sem_post resource");
}

//
// Don't change the code below (just use it!)
//

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_readlock(&lock);
	printf("read %d\n", value);
	rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_writelock(&lock);
	value++;
	printf("write %d\n", value);
	rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
	Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
	Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}