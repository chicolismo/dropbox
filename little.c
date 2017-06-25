#include <stdio.h>

int main()
{
	int n = 1;
	if(*(char *)&n == 1)
		printf("alooo\n");
}
