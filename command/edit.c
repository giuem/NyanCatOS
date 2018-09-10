#include "stdio.h"
#include "sys/const.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: edit FILE\n");
        return 1;
    }
    char const *filename = argv[1];

    struct stat st;

    if (stat(filename, &st) < 0) {
        printf("edit: fail to open %s\n", filename);
        return -1;
    }

    if (st.st_mode != I_REGULAR) {
        printf("edit: %s is not a file\n", filename);
        return -1;
    }

    int fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        printf("edit: fail to open %s\n", filename);
        return -1;
    }

    printf("Please enter content: \n");

    const int len = 128;
    char buf[len];
    int n = read(0, buf, len);
    buf[n] = 0;

    write(fd, buf, n + 1);
    printf("done!\n");
    close(fd);
    return 0;
}
