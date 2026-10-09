#include <stdio.h>

int main() {
    int *p = NULL;

    printf("Pointer address: %p\n", (void *)p);

    *p = 10;   // Dereference NULL pointer

    return 0;
}