#include <stdio.h>
#include "enumTest.hpp"
int check(Test t1, Test t2)
{
	printf("%d %d\n", t1, t2);
}

int main(int argc, char const *argv[])
{
	check(A, B);
	check(A, C);
	check(B, C);
	
	return 0;
}