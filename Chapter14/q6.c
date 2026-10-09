#include <stdio.h>
#include <stdlib.h>

int main() {
    int *data = malloc(100 * sizeof(int));

    if (data == NULL) {
        return 1;
    }

    data[0] = 42;

    free(data);

    printf("%d\n", data[0]);   // Use after free

    return 0;
}