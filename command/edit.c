#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: edit [filename]\n");
        return 1;
    }
    char const *filename = argv[1];
    int fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        printf("fail to open %s\n", filename);
        return 1;
    }

    printf("enter content: \n");

    const int len = 128;
    char buf[len];
    int n = read(0, buf, len);
    buf[n] = 0;

    write(fd, buf, n + 1);
    printf("done!\n");
    close(fd);
    return 0;
}
