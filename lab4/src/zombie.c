#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Создание дочернего процесса
    if ((pid = fork()) == 0) {
        // Дочерний процесс
        printf("Дочерний процесс (PID: %d) завершился.\n", getpid());
        exit(0);  // Завершение дочернего процесса
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс (PID: %d) спит 10 секунд...\n", getpid());
        sleep(5);  // Ожидание, чтобы дочерний процесс стал зомби
        printf("Родительский процесс завершился.\n");
    } else {
        // Ошибка при создании процесса
        printf("Ошибка fork");
        return 1;
    }

    return 0;
}
