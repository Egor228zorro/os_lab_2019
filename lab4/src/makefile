# Имя выходного файла
TARGET = parallel_min_max

# Исходные файлы
SRC = utils.c find_min_max.c parallel_min_max.c

# Объектные файлы
OBJ = $(SRC:.c=.o)

# Компилятор и флаги
CC = gcc
CFLAGS = -I.

# Правило для линковки
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# Правило для компиляции .c в .o
%.o: %.c
	$(CC) -c $< $(CFLAGS)

# Очистка
clean:
	rm -f $(OBJ) $(TARGET)
