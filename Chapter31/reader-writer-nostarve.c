#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
    sem_t reader_guard;
    sem_t resource;
    int readers;

    /* Morris's two-turnstile lock for admission. */
    sem_t entry_guard;
    sem_t first_gate;
    sem_t second_gate;
    int room1;
    int room2;
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

static void post_sem(sem_t *sem) {
    check_sem(sem_post(sem), "sem_post");
}

static void enter_admission(rwlock_t *rw) {
    wait_sem(&rw->entry_guard);
    rw->room1++;
    post_sem(&rw->entry_guard);

    wait_sem(&rw->first_gate);
    rw->room2++;

    wait_sem(&rw->entry_guard);
    rw->room1--;

    if (rw->room1 == 0) {
        /* Close the first gate and let the current group proceed. */
        post_sem(&rw->entry_guard);
        post_sem(&rw->second_gate);
    } else {
        post_sem(&rw->entry_guard);
        post_sem(&rw->first_gate);
    }

    wait_sem(&rw->second_gate);
    rw->room2--;
}

static void leave_admission(rwlock_t *rw) {
    if (rw->room2 == 0) {
        /* The current group has finished. Admit the next group. */
        post_sem(&rw->first_gate);
    } else {
        post_sem(&rw->second_gate);
    }
}

void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    rw->room1 = 0;
    rw->room2 = 0;

    check_sem(sem_init(&rw->reader_guard, 0, 1), "sem_init reader_guard");
    check_sem(sem_init(&rw->resource, 0, 1), "sem_init resource");
    check_sem(sem_init(&rw->entry_guard, 0, 1), "sem_init entry_guard");
    check_sem(sem_init(&rw->first_gate, 0, 1), "sem_init first_gate");
    check_sem(sem_init(&rw->second_gate, 0, 0), "sem_init second_gate");
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    enter_admission(rw);

    wait_sem(&rw->reader_guard);
    if (++rw->readers == 1) {
        /* The first reader acquires the resource for the reader group. */
        wait_sem(&rw->resource);
    }
    post_sem(&rw->reader_guard);

    leave_admission(rw);
}

void rwlock_release_readlock(rwlock_t *rw) {
    wait_sem(&rw->reader_guard);
    if (--rw->readers == 0) {
        post_sem(&rw->resource);
    }
    post_sem(&rw->reader_guard);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    enter_admission(rw);
    wait_sem(&rw->resource);
    leave_admission(rw);
}

void rwlock_release_writelock(rwlock_t *rw) {
    post_sem(&rw->resource);
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