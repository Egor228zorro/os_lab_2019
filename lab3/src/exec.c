#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // Создаем дочерний процесс с помощью fork
    pid_t pid = fork();

    if (pid < 0) {
        // Если fork вернул отрицательное значение, значит произошла ошибка
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Код выполняется в дочернем процессе
        printf("Запускаем sequential_min_max в дочернем процессе...\n");

        // Вызываем execl для замены текущего процесса на sequential_min_max
        if (execl("./sequential_min_max", "sequential_min_max", "3", "50", NULL) == -1) {
            perror("execl failed");
            return 1;
        }
    } else {
        // Код выполняется в родительском процессе
        printf("Запущен процесс sequential_min_max с PID: %d\n", pid);

        // Ожидание завершения дочернего процесса
        wait(NULL);
        printf("Процесс sequential_min_max завершен.\n");
    }

    return 0;
}
