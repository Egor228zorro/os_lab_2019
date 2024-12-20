#include "swap.h"

void Swap(char *left, char *right)
{
	char temp = *left; // Сохраняем значение, на которое указывает left
    *left = *right;    // Копируем значение из right в left
    *right = temp;     // Восстанавливаем значение left в переменную right
}
