#include <list>

class PriorityQueue<class Type, bool (*compare)(Type *t1, Type *t2)>
{
public:
	int size();

	void pop();
	Type *top();
	void push(Type *thread);
	void erase(Type *thread);

private:
	std::list<Type*> _ready;
};