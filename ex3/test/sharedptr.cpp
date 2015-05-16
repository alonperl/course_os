#include "sharedptr.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	shared_ptr<Te> a1(new Te);
	cout << a1.use_count() << endl;
	shared_ptr<Te> a2(new Te);
	cout << a1.use_count() << endl;
	shared_ptr<Te> a3(new Te);
	cout << a1.use_count() << endl;


	return 0;
}