#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEM_SIZE 10

int main(int argc, char *argv[]) {
    char *addr;
    int fd;

    // Проверяем аргументы командной строки
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "Usage: %s file [new-value]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Открываем файл, переданный через первый аргумент, в режиме чтения-записи */
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Создаем разделяемое отображение
    addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* Дескриптор 'fd' больше не нужен */
    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    /* Выводим строку по адресу отображения. Для безопасности ограничиваем вывод MEM_SIZE байтами */
    printf("Current string=%.*s\n", MEM_SIZE, addr);

    /* Проверяем длину строки, переданной через второй аргумент */
    if (argc > 2) {
        if (strlen(argv[2]) >= MEM_SIZE) {
            fprintf(stderr, "'new-value' too large\n");
            exit(EXIT_FAILURE);
        }

        /* Заполняем участок памяти нулями */
        memset(addr, 0, MEM_SIZE);
        /* Записываем строку, переданную через второй аргумент */
        strncpy(addr, argv[2], MEM_SIZE - 1);

        // Синхронизируем изменения с диском
        if (msync(addr, MEM_SIZE, MS_SYNC) == -1) {
            perror("msync");
            exit(EXIT_FAILURE);
        }

        printf("Copied \"%s\" to shared memory\n", argv[2]);
    }

    exit(EXIT_SUCCESS);
}