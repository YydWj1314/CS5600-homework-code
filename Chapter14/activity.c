#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *username = malloc(20);

    if (username == NULL) {
        return 1;
    }

    strcpy(username, "Daniel");

    free(username);

    // Bug: using memory after it has been freed
    printf("User: %s\n", username);

    return 0;
}