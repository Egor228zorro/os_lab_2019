#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct {
    int start;
    int end;
    int mod;
    long long* result;
    pthread_mutex_t* mutex;
} ThreadArgs;

void* partial_factorial(void* args) {
    ThreadArgs* targs = (ThreadArgs*)args;
    long long partial_result = 1;

    for (int i = targs->start; i <= targs->end; ++i) {
        partial_result = (partial_result * i) % targs->mod;
    }

    pthread_mutex_lock(targs->mutex);
    *(targs->result) = (*(targs->result) * partial_result) % targs->mod;
    pthread_mutex_unlock(targs->mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    int k = -1, pnum = -1, mod = -1;

    const struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while (1) {
        int c = getopt_long(argc, argv, "k:p:m:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 'k': k = atoi(optarg); break;
            case 'p': pnum = atoi(optarg); break;
            case 'm': mod = atoi(optarg); break;
            default: return 1;
        }
    }

    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Invalid arguments. Example usage: %s -k 10 --pnum=4 --mod=10\n", argv[0]);
        return 1;
    }

    pthread_t threads[pnum];
    ThreadArgs thread_args[pnum];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    long long result = 1;
    int chunk_size = k / pnum;
    int remainder = k % pnum;

    for (int i = 0; i < pnum; ++i) {
        thread_args[i].start = i * chunk_size + 1;
        thread_args[i].end = (i + 1) * chunk_size + (i == pnum - 1 ? remainder : 0);
        thread_args[i].mod = mod;
        thread_args[i].result = &result;
        thread_args[i].mutex = &mutex;

        if (pthread_create(&threads[i], NULL, partial_factorial, &thread_args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < pnum; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    pthread_mutex_destroy(&mutex);

    printf("Result: %lld\n", result);
    return 0;
}
