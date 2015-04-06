#include <queue>
#include <cstdlib>

class testpq
{
public:
  	std::priority_queue<Test, std::vector<Test>, TestComparator> readyQueue;
};

class Test
{
public:
	int age;
	std::string name;
};

struct TestComparator
{
	bool operator()(Test &t1, Test &t2)
	{
        return t1.age < t2.age;
    }
};