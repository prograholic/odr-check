#include <iostream>

enum Foo
{
	e1,
	e2,
	eCount
};

const char* GetMsg(Foo x);

int main()
{
	std::cout << GetMsg(e2) << std::endl;
	return 0;
}
