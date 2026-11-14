#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

static sem_t s1, s2;

static void check_sem(int result, const char *operation)
{
    if (result == -1) {
        perror(operation);
        exit(EXIT_FAILURE);
    }
}

static void wait_for_arrival(sem_t *s)
{
    int result;

    /* Retry if a signal interrupts the wait. */
    do {
        result = sem_wait(s);
    } while (result == -1 && errno == EINTR);

    check_sem(result, "sem_wait");
}

void *child_1(void *arg)
{
    (void)arg;
    printf("child 1: before\n");

    check_sem(sem_post(&s1), "sem_post");  /* Child 1 has arrived. */
    wait_for_arrival(&s2);               /* Wait for child 2. */

    printf("child 1: after\n");
    return NULL;
}

void *child_2(void *arg)
{
    (void)arg;
    sleep(1);  /* Delay child 2 to test that child 1 waits. */
    printf("child 2: before\n");

    check_sem(sem_post(&s2), "sem_post");  /* Child 2 has arrived. */
    wait_for_arrival(&s1);               /* Wait for child 1. */

    printf("child 2: after\n");
    return NULL;
}

int main(void)
{
    pthread_t p1, p2;

    printf("parent: begin\n");

    /* Initially, neither child has announced its arrival. */
    check_sem(sem_init(&s1, 0, 0), "sem_init");
    check_sem(sem_init(&s2, 0, 0), "sem_init");

    Pthread_create(&p1, NULL, child_1, NULL);
    Pthread_create(&p2, NULL, child_2, NULL);

    /* Join both children before destroying either semaphore. */
    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);

    check_sem(sem_destroy(&s1), "sem_destroy");
    check_sem(sem_destroy(&s2), "sem_destroy");

    printf("parent: end\n");
    return EXIT_SUCCESS;
}