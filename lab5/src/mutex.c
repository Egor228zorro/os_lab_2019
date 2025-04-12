/********************************************************
 * Пример работы с мьютексами в многопоточном приложении
 *
 * - создание потоков
 * - разделение общей переменной
 * - синхронизацию с помощью мьютекса
 * - состояние гонки (при отключенном мьютексе)
 */
 #include <errno.h>   // Для работы с кодами ошибок
 #include <pthread.h> // Функции работы с потоками POSIX
 #include <stdio.h>
 #include <stdlib.h>
 
 void do_one_thing(int *);   // функция для 1 потока
 void do_another_thing(int *); // функция для 2 потока
 void do_wrap_up(int);        // функция для завершения
 int common = 0;              /* общая переменная для 2 потоков */
 int r1 = 0, r2 = 0, r3 = 0;
 pthread_mutex_t mut =
     PTHREAD_MUTEX_INITIALIZER; // Инициализация мьютекса с помощью макроса
 
 int main() {
   pthread_t thread1, thread2; // идентификаторы потоков
 
   // Создание первого потока
   if (pthread_create(&thread1, NULL, (void *)do_one_thing,
                      (void *)&common) != 0) {
     perror("pthread_create"); 
     exit(1);                  
   }
 
   // Создание второго потока
   if (pthread_create(&thread2, NULL, (void *)do_another_thing,
                      (void *)&common) != 0) {
     perror("pthread_create"); 
     exit(1);                  
   }
 
   // Ожидание завершения первого потока
   if (pthread_join(thread1, NULL) != 0) {
     perror("pthread_join"); 
     exit(1);                  
   }
 
   // Ожидание завершения второго потока
   if (pthread_join(thread2, NULL) != 0) {
     perror("pthread_join"); 
     exit(1);                  
   }
 
   // Вызов функции завершения
   do_wrap_up(common);
 
   return 0; // Успешное завершение программы
 }
 
 // Функция, выполняемая первым потоком
 void do_one_thing(int *pnum_times) {
   int i, j, x;
   unsigned long k;
   int work;
   for (i = 0; i < 50; i++) {
    pthread_mutex_lock(&mut); // Захват мьютекса (закомментировано ,если нужно для демонстрации состояния гонки)
     printf("doing one thing\n"); // Вывод сообщения
     work = *pnum_times;           // Чтение значения общей переменной
     printf("counter = %d\n", work); // Вывод значения счетчика
     work++;                       /* увеличение, но не запись */
     for (k = 0; k < 500000; k++)
       ;                     /* длинный цикл */
     *pnum_times = work;   /* запись обратно */
     pthread_mutex_unlock(&mut); // Освобождение мьютекса (закомментировано)
   }
 }
 
 // Функция, выполняемая вторым потоком
 void do_another_thing(int *pnum_times) {
   int i, j, x;
   unsigned long k;
   int work;
   for (i = 0; i < 50; i++) {
     pthread_mutex_lock(&mut); // Захват мьютекса (надо закометить,если нужно состояние гонки)
     printf("doing another thing\n"); 
     work = *pnum_times;           // Чтение значения общей переменной
     printf("counter = %d\n", work); // Вывод значения счетчика
     work++;                       /* увеличение, но не запись */
     for (k = 0; k < 500000; k++)
       ;                     /* длинный цикл */
     *pnum_times = work;   /* запись обратно */
      pthread_mutex_unlock(&mut); // Освобождение мьютекса (надо закометить,если нужно состояние гонки)
   }
 }
 
 // Функция завершения
 void do_wrap_up(int counter) {
   int total;
   printf("All done, counter = %d\n", counter); // Вывод итогового значения счетчика
 }
