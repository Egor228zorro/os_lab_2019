#ifndef SUM_H
#define SUM_H

struct SumArgs {
    int *array;
    int begin;
    int end;
};
unsigned long long int Sum(const struct SumArgs *args);

#endif // SUM_H
