all: t_mmap p_mmap

t_mmap: t_mmap.c
	gcc -Wall -Wextra -std=gnu11 -o t_mmap t_mmap.c

p_mmap: p_mmap.c
	gcc -Wall -Wextra -std=gnu11 -o p_mmap p_mmap.c

clean:
	rm -f t_mmap p_mmap