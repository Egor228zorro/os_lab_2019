#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = -1;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"seed", required_argument, 0, 0},
                                          {"array_size", required_argument, 0, 0},
                                          {"pnum", required_argument, 0, 0},
                                          {"by_files", no_argument, 0, 'f'},
                                          {"timeout", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        break;

                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
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

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] \n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    int active_child_processes = 0;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Создание pipe для передачи данных от дочерних процессов
    int pipes[pnum][2];
    for (int i = 0; i < pnum; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            free(array);
            return 1;
        }
    }

    pid_t child_pids[pnum]; // Массив для хранения PID дочерних процессов

    // Создание дочерних процессов
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        child_pids[i] = child_pid; // Сохраняем PID
        if (child_pid >= 0) {
            active_child_processes += 1;
            if (child_pid == 0) {
                // Код дочернего процесса
                close(pipes[i][0]); // Закрыть чтение
                int start_idx = (array_size / pnum) * i;
                int end_idx = (i == pnum - 1) ? array_size : (array_size / pnum) * (i + 1);

                struct MinMax min_max = GetMinMax(array, start_idx, end_idx);

                if (with_files) {
                    char filename[20];
                    sprintf(filename, "result_%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    if (file) {
                        fprintf(file, "min: %d\nmax: %d\n", min_max.min, min_max.max);
                        fclose(file);
                    } else {
                        perror("fopen");
                    }
                } else {
                    write(pipes[i][1], &min_max, sizeof(min_max));
                }

                close(pipes[i][1]); // Закрыть запись
                free(array);
                exit(0);
            }
        } else {
            perror("fork");
            free(array);
            return 1;
        }
    }

    // Установка таймаута, если он задан
    if (timeout > 0) {
        sleep(timeout);
        printf("Timeout reached. Killing child processes.\n");
        for (int i = 0; i < pnum; i++) {
            if (child_pids[i] > 0) {
                kill(child_pids[i], SIGKILL);
            }
        }
    }

    // Ожидание завершения всех дочерних процессов
    int finished_processes = 0;
    while (finished_processes < pnum) {
        int status;
        pid_t pid = wait(&status);
        if (pid > 0) {
            active_child_processes -= 1;
            finished_processes++;
        } else {
            perror("wait"); // Сообщаем об ошибке, но не прерываем цикл.
        }
    }

    // Сбор результатов
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[20];
            sprintf(filename, "result_%d.txt", i);
            FILE *file = fopen(filename, "r");
            if (file) {
                fscanf(file, "min: %d\nmax: %d\n", &min, &max);
                fclose(file);
            } else {
                perror("fopen");
            }
        } else {
            close(pipes[i][1]); // Закрыть запись
            ssize_t bytes_read = read(pipes[i][0], &min_max, sizeof(min_max));
            close(pipes[i][0]); // Закрыть чтение
            if (bytes_read > 0) {
                min = min_max.min;
                max = min_max.max;
            } else {
                // Если не удалось прочитать данные из pipe, устанавливаем значения по умолчанию
                min = INT_MAX;
                max = INT_MIN;
            }
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    return 0;
}





