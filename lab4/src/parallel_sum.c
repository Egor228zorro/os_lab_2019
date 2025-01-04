#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h" 
#include "sum.h"
#include <pthread.h>
#include <getopt.h>
void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
    unsigned long long int *result = malloc(sizeof(unsigned long long int));
    *result = Sum(sum_args); 
    return (void *)result;
}

int main(int argc, char **argv) {
 
  uint32_t threads_num = -1;
  uint32_t seed = -1;
  uint32_t array_size = -1;

  

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"threads_num", required_argument, 0, 0},
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

      int option_index = 0;
      int c = getopt_long(argc, argv, "", options, &option_index);

      if (c == -1) break;

      switch (c) {
          case 0:
              switch (option_index) {
                  case 0:
                      threads_num = atoi(optarg);
                      if (threads_num <= 0) {
                          printf("threads_num must be a positive number\n");
                          return 1;
                      }
                      break;
                  case 1:
                      seed = atoi(optarg);
                      if (seed <= 0) {
                          printf("seed must be a positive number\n");
                          return 1;
                      }
                      break;
                  case 2:
                      array_size = atoi(optarg);
                      if (array_size <= 0) {
                          printf("array_size must be a positive number\n");
                          return 1;
                      }
                      break;
                  default:
                      printf("Index %d is out of options\n", option_index);
              }
              break;

          case '?':
              break;

          default:
              printf("getopt returned character code 0%o?\n", c);
      }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || threads_num == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" --timeout \"num\"\n",
           argv[0]);
    return 1;
  }

  pthread_t threads[threads_num];
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct SumArgs args[threads_num];
  int chunk_size = array_size / threads_num;

   clock_t start_time = clock();

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk_size;
    args[i].end = (i + 1) * chunk_size;
    if (i == threads_num - 1) {
        args[i].end = array_size; 
    }

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
        printf("Error: pthread_create failed!\n");
        return 1;
    }
  }

  unsigned long long int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
      unsigned long long int *sum;
      pthread_join(threads[i], (void **)&sum);
      total_sum += *sum;
      //free(sum);
  }

  clock_t end_time = clock();
  double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  free(array);
  printf("Total: %llu\n", total_sum);
  printf("Time taken to calculate sum: %.6f seconds\n", time_taken);
  return 0;
}
