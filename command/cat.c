#include "stdio.h"
#include "sys/const.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: cat FILE [FILE]\n");
        return 1;
    }
    char const *filename;
    int i;
    for (i = 1; i < argc; ++i) {
        filename = argv[i];
        struct stat st;
    
        if (stat(filename, &st) < 0) {
            printf("cat: fail to open %s\n", filename);
            return -1;
        }

        if (st.st_mode != I_REGULAR) {
            printf("cat: %s is not a file\n", filename);
            return -1;
        }

        int fd = open(filename, O_RDWR);
        if (fd == -1)
        {
            printf("cat: fail to open %s\n", filename);
            return -1;
        }

        const int len = 128;
        char buf[len];
        int n = read(fd, buf, len);
        buf[n] = 0;

        printf("%s\n", buf);

        close(fd);
    }
    return 0;
}
