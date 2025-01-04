#include "sum.h"

unsigned long long int Sum(const struct SumArgs *args) {
    unsigned long long int sum = 0;
    for (int i = args->begin; i < args->end; i++) {
        sum += args->array[i];
    }
    return sum;
}
