#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <MB>\n", argv[0]);
        return 1;
    }

    int mb = atoi(argv[1]);

    long size = (long)mb * 1024 * 1024;

    char *array = malloc(size);

    if (array == NULL)
    {
        printf("malloc failed\n");
        return 1;
    }

    printf("PID = %d\n", getpid());

    while (1)
    {
        for (long i = 0; i < size; i++)
        {
            array[i]++;
        }
    }

    return 0;
}