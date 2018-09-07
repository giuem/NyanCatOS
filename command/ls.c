#include "stdio.h"

int main(int argc, char const *argv[])
{
    int fd = open(".", O_RDWR);
    if (fd == -1) {
        return 1;
    }
    return getdents(fd);
}
