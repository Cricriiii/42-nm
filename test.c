#include <stdio.h>
#include <stdlib.h>
#include <time.h>

size_t size = 1000;

struct data {
    int data;
    struct data *next;
};

struct data* last;

__attribute((constructor)) void init_program(void ) {
    srand(time(nullptr));
}

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "Need one argument\n");
        return 1;
    }

    struct data* list = nullptr;
    struct data* last = list;

    int iter = atoi(argv[1]);
    for (int i = 0; i < iter; ++i) {
        struct data* tmp = calloc(1, sizeof(struct data));
        tmp->data = i;

        if (!list) {
            list = tmp;
            last = list;
        } else {
            last->next = tmp;
            last = tmp;
        }
    }

    printf("Allocated: %d data structures\n", iter);

    while (list) {
        last = list->next;
        free(list);
        list = last;
    }

    return 0;
}