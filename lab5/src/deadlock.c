#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Инициализируем два мьютекса
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// Функция для первого потока
void* thread_func1(void* arg) {
    printf("Поток 1: Пытаюсь заблокировать Мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 1: Заблокировал Мьютекс 1.\n");

    // Искусственная задержка для повышения вероятности deadlock
    sleep(1);

    printf("Поток 1: Пытаюсь заблокировать Мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 1: Заблокировал Мьютекс 2.\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    printf("Поток 1: Освободил Мьютекс 1 и Мьютекс 2.\n");
    return NULL;
}

// Функция для второго потока
void* thread_func2(void* arg) {
    printf("Поток 2: Пытаюсь заблокировать Мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 2: Заблокировал Мьютекс 2.\n");

    // Искусственная задержка для повышения вероятности deadlock
    sleep(1);

    printf("Поток 2: Пытаюсь заблокировать Мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 2: Заблокировал Мьютекс 1.\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    printf("Поток 2: Освободил Мьютекс 1 и Мьютекс 2.\n");
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Создаём два потока
    pthread_create(&thread1, NULL, thread_func1, NULL);
    pthread_create(&thread2, NULL, thread_func2, NULL);

    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Уничтожаем мьютексы
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    printf("Главный поток: Завершение программы.\n");
    return 0;
}
