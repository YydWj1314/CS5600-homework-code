#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    int size;
    int capacity;
} Vector;

void initVector(Vector *v) {
    v->size = 0;
    v->capacity = 2;
    v->data = malloc(v->capacity * sizeof(int));

    if (v->data == NULL) {
        perror("malloc");
        exit(1);
    }
}

void pushBack(Vector *v, int value) {
    if (v->size == v->capacity) {
        v->capacity *= 2;

        int *temp = realloc(v->data, v->capacity * sizeof(int));

        if (temp == NULL) {
            perror("realloc");
            free(v->data);
            exit(1);
        }

        v->data = temp;
    }

    v->data[v->size] = value;
    v->size++;
}

void printVector(Vector *v) {
    for (int i = 0; i < v->size; i++) {
        printf("%d ", v->data[i]);
    }
    printf("\n");
}

void freeVector(Vector *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int main() {
    Vector v;

    initVector(&v);

    for (int i = 0; i < 20; i++) {
        pushBack(&v, i);
    }

    printVector(&v);

    freeVector(&v);

    return 0;
}