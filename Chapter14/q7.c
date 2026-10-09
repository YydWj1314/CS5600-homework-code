#include <stdio.h>
#include <stdlib.h>

int main() {
    int *data = malloc(100 * sizeof(int));

    if (data == NULL) {
        return 1;
    }

    free(data + 10);   // Invalid free

    return 0;
}