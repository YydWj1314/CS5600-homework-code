#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

static sem_t s;

void *child(void *arg)
{
    (void)arg;

    sleep(1);
    printf("child\n");

    /* Signal the parent after the child finishes its work. */
    if (sem_post(&s) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

    return NULL;
}

int main(void)
{
    pthread_t p;

    printf("parent: begin\n");

    /* Start at 0 so the parent waits for the child's signal. */
    if (sem_init(&s, 0, 0) == -1) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    Pthread_create(&p, NULL, child, NULL);

    /* Retry if a signal interrupts the wait. */
    while (sem_wait(&s) == -1) {
        if (errno != EINTR) {
            perror("sem_wait");
            return EXIT_FAILURE;
        }
    }

    printf("parent: end\n");

    Pthread_join(p, NULL);
    if (sem_destroy(&s) == -1) {
        perror("sem_destroy");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}