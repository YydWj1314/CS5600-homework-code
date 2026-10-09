#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(100 * sizeof(int));

    if (p == NULL) {
        return 1;
    }

    p[0] = 10;

    printf("p[0] = %d\n", p[0]);

    // Forgot to call free(p)

    return 0;
}