#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

/*
пример использования mmap в режиме MAP_ANONYMOUS | MAP_PRIVATE
будет создан сыновний процесс, родитель и сын будут работать
со своими данными
 */

/*
использование ./mmap4
 */
int main(int argc, char *argv[])
{
    int pagesize = getpagesize();
    printf("page size: %d\n", pagesize);

    void *mp = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (mp == MAP_FAILED) {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }
    printf("ptr: %p\n", mp);
    *(int*) mp = 0x12345678;
    fork();
    printf("pid: %d\n", getpid());
    int *data = mp;
    while (1) {
        printf("%d, %d, %d, %d\n", getpid(), data[0], data[1], data[2]);
        struct timeval tt;
        gettimeofday(&tt, NULL);
        data[0] = getpid();
        data[1] = tt.tv_sec;
        data[2] = tt.tv_usec;
        sleep(1);
    }
}
