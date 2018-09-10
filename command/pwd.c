#include "type.h"
#include "stdio.h"

int main(int argc, char * argv[])
{
	char pwd[128];
	getpwd(pwd, 128);
	printf("%s\n", pwd);
	return 0;
}
