#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
    struct MinMax min_max;
    min_max.min = INT_MAX; // Инициализация минимального значения
    min_max.max = INT_MIN; // Инициализация максимального значения

    // Проход по массиву в указанном диапазоне
    for (unsigned int i = begin; i < end; i++) {
        if (array[i] < min_max.min) {
            min_max.min = array[i]; // Обновление минимального значения
        }
        if (array[i] > min_max.max) {
            min_max.max = array[i]; // Обновление максимального значения
        }
    }

    return min_max;
}