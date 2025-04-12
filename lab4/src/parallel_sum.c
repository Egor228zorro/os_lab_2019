#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h" 
#include "sum.h"
#include <pthread.h>
#include <getopt.h>
void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args; // Приведение аргумента к нужному типу
    unsigned long long int *result = malloc(sizeof(unsigned long long int)); // Выделение памяти для результата
    if (result == NULL) {
        perror("malloc in ThreadSum"); // Обработка ошибки выделения памяти
        pthread_exit(NULL); // Завершение потока в случае ошибки
    }
    *result = Sum(sum_args); // Вычисление частичной суммы
    return (void *)result; // Возврат указателя на результат
}

int main(int argc, char **argv) {
    uint32_t threads_num = -1;   // Количество потоков
    uint32_t seed = -1;          // Начальное значение для генератора случайных чисел
    uint32_t array_size = -1;    // Размер массива

    // Разбор аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1; // Сохранение текущего индекса optind

        static struct option options[] = {
            {"threads_num", required_argument, 0, 0}, // Опция --threads_num
            {"seed", required_argument, 0, 0},        // Опция --seed
            {"array_size", required_argument, 0, 0},   // Опция --array_size
            {0, 0, 0, 0}                                // Конец массива опций
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index); // Разбор опции

        if (c == -1)
            break; // Выход из цикла, если опций больше нет

        switch (c) {
            case 0: // Обработка длинных опций
                switch (option_index) {
                    case 0: // --threads_num
                        threads_num = atoi(optarg); // Преобразование аргумента в число
                        if (threads_num <= 0) {
                            fprintf(stderr, "threads_num должно быть положительным числом\n");
                            return 1; // Выход с ошибкой
                        }
                        break;
                    case 1: // --seed
                        seed = atoi(optarg); // Преобразование аргумента в число
                        if (seed <= 0) {
                            fprintf(stderr, "начальное значение должно быть положительным числом\n");
                            return 1; 
                        }
                        break;
                    case 2: // --array_size
                        array_size = atoi(optarg); // Преобразование аргумента в число
                        if (array_size <= 0) {
                            fprintf(stderr, "значение array_size должно быть положительным числом\n");
                            return 1; // Выход с ошибкой
                        }
                        break;
                    default:
                        fprintf(stderr, "Index %d is out of options\n", option_index);
                }
                break;

            case '?': // Обработка неизвестных опций
                break;

            default:
                fprintf(stderr, "getopt возвращает код символа 0%o?\n", c);
        }
    }

    // Проверка наличия аргументов без опций
    if (optind < argc) {
        fprintf(stderr, "Имеет по крайней мере один аргумент без опции\n");
        return 1; 
    }

    // Проверка, что все необходимые аргументы заданы
    if (seed == -1 || array_size == -1 || threads_num == -1) {
        fprintf(stderr, "Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
        return 1; 
    }

    // Объявление массива потоков
    pthread_t threads[threads_num];
    // Выделение памяти для массива
    int *array = malloc(sizeof(int) * array_size);
    if (array == NULL) {
        perror("malloc for array"); // Обработка ошибки выделения памяти
        return 1; 
    }
    // Заполнение массива случайными числами
    GenerateArray(array, array_size, seed);

    // Объявление массива аргументов для потоков
    struct SumArgs args[threads_num];
    // Вычисление размера куска для каждого потока
    int chunk_size = array_size / threads_num;

    clock_t start_time = clock(); // Запись времени начала

    // Создание потоков
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;                               // Указатель на массив
        args[i].begin = i * chunk_size;                      // Начальный индекс
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size; // Конечный индекс (последний поток обрабатывает остаток)

        // Создание потока
        int create_result = pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i]);
        if (create_result) {
            fprintf(stderr, "Ошибка: не удалось создать pthread_create! Код ошибки: %d\n", create_result);
            free(array); // Освобождение памяти перед выходом
            return 1;    // Выход с ошибкой
        }
    }

    unsigned long long int total_sum = 0; // Общая сумма
    // Ожидание завершения потоков и суммирование результатов
    for (uint32_t i = 0; i < threads_num; i++) {
        unsigned long long int *sum;
        int join_result = pthread_join(threads[i], (void **)&sum); // Ожидание завершения потока
        if (join_result != 0) {
            fprintf(stderr, "Ошибка: не удалось выполнить pthread_join! Код ошибки: %d\n", join_result);
            // Обработка ошибки (например, выход из программы)
            free(array);
            return 1;
        }
        total_sum += *sum; // Добавление частичной суммы к общей сумме
        free(sum);          // Освобождение памяти, выделенной для результата потока
    }

    clock_t end_time = clock(); // Запись времени окончания
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC; // Вычисление времени выполнения

    free(array); // Освобождение памяти, выделенной для массива

    printf("Total: %llu\n", total_sum);                                  // Вывод общей суммы
    printf("Время, затраченное на вычисление суммы: %.6f секунд\n", time_taken); // Вывод времени выполнения

    return 0; 
}
