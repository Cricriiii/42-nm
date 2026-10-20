#include <stdio.h>
#include <stdlib.h>

size_t size = 1000;

int main(void) {

    void* ptr = malloc(size);

    printf("Allocated: %lu\n", size);

    free(ptr);

    return 0;
}