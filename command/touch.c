#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("Usage: touch <file1> [file2] ...\n");
        return -1;
    }
    int i;
    int fd;
    for (i = 1; i < argc; i++)
    {
        fd = open(argv[1], O_CREAT);
        if (fd == -1)
        {
            printf("fail to create file");
        }
        close(fd);
    }
    return 0;
}