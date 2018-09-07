#include "stdio.h"

int main(int argc, char const *argv[])
{
    struct time t;
    date(&t);
    printf("%d-%d-%dT%d:%d:%d", t.year, t.month, t.day, t.hour, t.minute, t.second);
    return 0;
}
