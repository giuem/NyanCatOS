#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("Usage: mv SOURCE DEST");
        return -1;
    }
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        printf("mv: %s not exists", argv[1]);
        return -1;
    }
    char buf[1024];
    int n = read(fd, buf, 1024);
    close(fd);

    fd = open(argv[2], O_RDWR|O_CREAT);
    if (fd == -1) {
        printf("mv: %s exists", argv[2]);
        return -1;
    }
    write(fd, buf, n+1);
    close(fd);

    // unlink(argv[1]);
    return 0;
}