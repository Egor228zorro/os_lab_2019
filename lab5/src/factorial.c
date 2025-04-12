#include <pthread.h> // Для работы с потоками
#include <stdio.h>   
#include <stdlib.h>  
#include <getopt.h>  
typedef struct {
    int start;           // Начало диапазона для вычисления частичного факториала
    int end;             // Конец диапазона для вычисления частичного факториала
    int mod;             // Модуль, по которому вычисляется факториал
    long long* result;   // Указатель на общую переменную для хранения результата
    pthread_mutex_t* mutex; // Указатель на мьютекс для синхронизации доступа к общей переменной
} ThreadArgs;

// Функция, выполняемая каждым потоком
void* partial_factorial(void* args) {
    ThreadArgs* targs = (ThreadArgs*)args; // Приведение аргумента к типу ThreadArgs*
    long long partial_result = 1;          // Инициализация частичного результата

    // Вычисление частичного факториала
    for (int i = targs->start; i <= targs->end; ++i) {
        partial_result = (partial_result * i) % targs->mod; // Вычисление произведения по модулю
    }

    // Захват мьютекса для безопасного обновления общей переменной result
    pthread_mutex_lock(targs->mutex);
    *(targs->result) = (*(targs->result) * partial_result) % targs->mod; // Обновление общей переменной result
    pthread_mutex_unlock(targs->mutex); // Освобождение мьютекса

    return NULL; // Поток завершает свою работу
}

int main(int argc, char* argv[]) {
    int k = -1, pnum = -1, mod = -1; // Инициализация переменных для аргументов командной строки

    // Определение структуры для описания длинных опций командной строки
    const struct option long_options[] = {
        {"k", required_argument, 0, 'k'},    // Опция --k с обязательным аргументом
        {"pnum", required_argument, 0, 'p'}, // Опция --pnum с обязательным аргументом
        {"mod", required_argument, 0, 'm'},  // Опция --mod с обязательным аргументом
        {0, 0, 0, 0}                          // Конец массива опций
    };

    int option_index = 0; // Индекс текущей опции
    // Разбор аргументов командной строки с использованием getopt_long
    while (1) {
        int c = getopt_long(argc, argv, "k:p:m:", long_options, &option_index); // Разбор опции
        if (c == -1) break; // Если getopt_long вернул -1, значит, опций больше нет

        switch (c) {
            case 'k': k = atoi(optarg); break;    // Опция -k: преобразование аргумента в целое число
            case 'p': pnum = atoi(optarg); break; // Опция -p: преобразование аргумента в целое число
            case 'm': mod = atoi(optarg); break;  // Опция -m: преобразование аргумента в целое число
            default: return 1;                     // Обработка неизвестной опции
        }
    }

    // Проверка валидности аргументов командной строки
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Invalid arguments. Example usage: %s -k 10 --pnum=4 --mod=10\n", argv[0]);
        return 1; // Выход с ошибкой
    }

    pthread_t threads[pnum];       // Объявление массива потоков
    ThreadArgs thread_args[pnum];    // Объявление массива аргументов для потоков
    pthread_mutex_t mutex;           // Объявление мьютекса
    pthread_mutex_init(&mutex, NULL); // Инициализация мьютекса

    long long result = 1; // Инициализация общей переменной для хранения результата
    int chunk_size = k / pnum; // Вычисление размера куска для каждого потока
    int remainder = k % pnum;  // Вычисление остатка от деления

    // Создание потоков
    for (int i = 0; i < pnum; ++i) {
        // Заполнение структуры аргументов для потока
        thread_args[i].start = i * chunk_size + 1; // Начало диапазона для текущего потока
        thread_args[i].end = (i + 1) * chunk_size + (i == pnum - 1 ? remainder : 0); // Конец диапазона для текущего потока (последний поток обрабатывает остаток)
        thread_args[i].mod = mod;             // Модуль
        thread_args[i].result = &result;       // Указатель на общую переменную result
        thread_args[i].mutex = &mutex;         // Указатель на мьютекс

        // Создание потока
        if (pthread_create(&threads[i], NULL, partial_factorial, &thread_args[i]) != 0) {
            perror("pthread_create"); 
            return 1;                
        }
    }

    // Ожидание завершения потоков
    for (int i = 0; i < pnum; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join"); 
            return 1;                
        }
    }

    pthread_mutex_destroy(&mutex); // Уничтожение мьютекса

    printf("Result: %lld\n", result); 
    return 0;                           
}