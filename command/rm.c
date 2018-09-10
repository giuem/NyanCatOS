#include "stdio.h"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("rm FILE1 [FILE2] [FILE3] ...");
        return 1;
    }

    int i;
    for (i = 1; i < argc; ++i) {
        if (unlink(argv[i]) != 0) {
            printf("rm: cannot remove %s\n", argv[i]);
        }
    }
    return 0;
}
