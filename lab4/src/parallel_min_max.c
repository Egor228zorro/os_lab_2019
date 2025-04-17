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
#include <signal.h> // нужна для работы с сигналами

#include <getopt.h> // для разбора аргументов командной строки

#include "find_min_max.h"
#include "utils.h"

pid_t child_pids[100]; // Массив для хранения PID'ов дочерних процессов
int pnum_global;       // Глобальное количество процессов (используется в обработчике сигналов)

/**
 * Обработчик сигнала SIGALRM
 * Срабатывает, если процессы не завершились за указанное время (--timeout)
 * Завершает оставшиеся дочерние процессы через SIGKILL
 */
 /*
 * 
 *
 * 1. Обработчик сигнала `timeout_handler` и вызов `alarm(timeout)`:
 *    - Позволяют завершить все дочерние процессы, если они не завершились в течение указанного времени (`--timeout`).
 *    - Это предотвращает "зависание" программы, если какой-либо из дочерних процессов работает слишком долго или завис.
 *    - В `timeout_handler` реализован вызов `kill(..., SIGKILL)` для принудительного завершения оставшихся процессов.
 *
 * 2. Использование `waitpid(..., WNOHANG)`:
 *    - Позволяет неблокирующе проверять завершение дочерних процессов.
 *    - Это позволяет родительскому процессу опрашивать статусы дочерних процессов в цикле без "заморозки".
 *    - В сочетании с `timeout_handler` гарантирует своевременное завершение всех дочерних процессов и сбор их результатов, даже если некоторые не ответили.
 */
void timeout_handler(int sig) {
    printf("Истек тайм-аут. Завершение дочерних процессов.\n");
    for (int i = 0; i < pnum_global; i++) {
        if (child_pids[i] > 0) { 
            kill(child_pids[i], SIGKILL); // Принудительное завершение дочернего процесса
        }
    }
}

int main(int argc, char **argv) {
    // Параметры командной строки
    int seed = -1;         // Значение генератора случайных чисел
    int array_size = -1;   // Размер массива
    int pnum = -1;         // Количество дочерних процессов
    bool with_files = false; // Флаг: использовать ли файлы для передачи данных
    int timeout = -1;      // Тайм-аут в секундах, после которого процессы будут убиты

    // Обработка аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                    case 4: timeout = atoi(optarg); break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                // неизвестная опция — ничего не делаем
                break;
            default:
                printf("getopt возвращает код символа 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Имеет по крайней мере один аргумент без опции\n");
        return 1;
    }

    // Проверка обязательных аргументов
    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] \n", argv[0]);
        return 1;
    }

    // Выделение памяти под массив
    int *array = malloc(sizeof(int) * array_size);
    if (array == NULL) {
        perror("malloc");
        return 1;
    }

    // Заполнение массива случайными числами
    GenerateArray(array, array_size, seed);

    int active_child_processes = 0; // Кол-во активных дочерних процессов

    struct timeval start_time;
    gettimeofday(&start_time, NULL); // Засекаем время начала

    int pipes[pnum][2]; // Массив pipe'ов для взаимодействия с процессами (если with_files == false)

    for (int i = 0; i < pnum; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            free(array);
            return 1;
        }
    }

    pnum_global = pnum; // Сохраняем для обработчика сигналов

    // Запуск дочерних процессов
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        child_pids[i] = child_pid;

        if (child_pid >= 0) {
            active_child_processes++;
            if (child_pid == 0) {
                // Дочерний процесс

                close(pipes[i][0]); // Закрываем чтение

                // Вычисляем границы обработки массива
                int start_idx = (array_size / pnum) * i;
                int end_idx = (i == pnum - 1) ? array_size : (array_size / pnum) * (i + 1);

                // Вычисляем min и max
                struct MinMax min_max = GetMinMax(array, start_idx, end_idx);

                if (with_files) {
                    // Запись результатов в файл
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
                    // Передача данных через pipe
                    write(pipes[i][1], &min_max, sizeof(min_max));
                }

                close(pipes[i][1]); // Закрываем pipe

                sleep(timeout); // Имитация задержки работы процесса (показывает, как работает тайм-аут)
                free(array);
                exit(0); // Завершение дочернего процесса
            }
        } else {
            perror("fork");
            free(array);
            return 1;
        }
    }

    // Устанавливаем сигнал SIGALRM на случай тайм-аута
    if (timeout > 0) {
        signal(SIGALRM, timeout_handler); // Назначаем обработчик сигнала
        alarm(timeout); // Устанавливаем таймер
    }

    int finished_processes = 0;

    // Цикл ожидания завершения дочерних процессов
    // Используем waitpid с флагом WNOHANG, чтобы не блокироваться
    while (finished_processes < pnum) {
        for (int i = 0; i < pnum; i++) {
            if (child_pids[i] <= 0) continue;

            int status;
            pid_t result = waitpid(child_pids[i], &status, WNOHANG);
            if (result > 0) {
                active_child_processes--;
                finished_processes++;
                child_pids[i] = 0; // Помечаем как завершенный
            } else if (result < 0) {
                perror("waitpid");
            }
        }
    }

    // Итоговая структура для хранения минимального и максимального значения
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    // Сбор результатов от дочерних процессов
    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            // Чтение из файла
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
            // Чтение из pipe
            close(pipes[i][1]); // Закрыть запись
            ssize_t bytes_read = read(pipes[i][0], &min_max, sizeof(min_max));
            close(pipes[i][0]); // Закрыть чтение
            if (bytes_read > 0) {
                min = min_max.min;
                max = min_max.max;
            } else {
                min = INT_MAX;
                max = INT_MIN;
            }
        }

        // Обновление общего min и max
        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    // Засекаем время окончания
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    // Вычисляем общее затраченное время в миллисекундах
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array); // Освобождение памяти

    // Вывод финальных результатов
    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Затраченное время: %fms\n", elapsed_time);

    return 0;
}






