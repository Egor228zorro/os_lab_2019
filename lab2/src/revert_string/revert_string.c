#include "revert_string.h"
#include <string.h>
void RevertString(char *str)
{
    int len = strlen(str);  // Находим длину строки c помощью стандартная библиотечной функции strlen
    int i = 0, j = len - 1;


    while (i < j)
    {
        char temp = str[i];  // Сохраняем символ с позиции i
        str[i] = str[j];     // Копируем символ с позиции j на позицию i
        str[j] = temp;       // Копируем сохраненный символ на позицию j

        i++;  // Перемещаем указатель i вправо
        j--;  // Перемещаем указатель j влево
    }
}


