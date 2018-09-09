#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("Usage: mkdir [dirname]\n");
        return -1;
    }
    const char * path = argv[1];
    int result = mkdir(path, O_CREAT);
    if (result == -1) {
        printf("directory %s exists\n", path);
    }

    return 0;
}
