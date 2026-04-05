#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
    char *addr;
    int fd;
    struct stat sb;
    off_t offset, pa_offset;
    size_t length;
    ssize_t s;

    /* Должно быть два или три аргумента: имя файла, смещение от начала, и необязательно длина */
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s file offset [length]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Открываем файл */
    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");

    /* Получаем размер файла */
    if (fstat(fd, &sb) == -1)
        handle_error("fstat");

    /* Преобразуем смещение в число */
    offset = atoi(argv[2]);

    /* Смещение для mmap() должно быть выровнено по границе страницы */
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }

    /* Преобразуем длину в число */
    if (argc == 4) {
        length = atoi(argv[3]);
        /* Если длина фрагмента выходит за границы файла, то выводим до конца файла */
        if (offset + length > sb.st_size)
            length = sb.st_size - offset;
    } else {
        /* Если длина не задана, то выводим до конца файла */
        length = sb.st_size - offset;
    }

    /* Создаем отображение */
    addr = mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, fd, pa_offset);
    if (addr == MAP_FAILED)
        handle_error("mmap");

    /* Выводим на экран */
    s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
    if (s != (ssize_t)length) {
        if (s == -1)
            handle_error("write");
        fprintf(stderr, "partial write");
        exit(EXIT_FAILURE);
    }

    /* Удаляем отображение */
    munmap(addr, length + offset - pa_offset);
    
    /* Закрываем файл */
    close(fd);

    exit(EXIT_SUCCESS);
}