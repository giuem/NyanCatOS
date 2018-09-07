#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("rm <file1> [file2] ...");
        return 1;
    }

    int i;
    for (i = 1; i < argc; ++i) {
        if (unlink(argv[i]) != 0) {
            printf("cannot remove %s\n", argv[i]);
        }
    }
    return 0;
}
