#include <iostream>

struct A {
	int x;
};

struct B {
	int y;
};

struct C {
	B b;
	A a;
};

void Modify(C& c);

int main()
{
	C c = {{0}, {0}};
	Modify(c);
	std::cout << "a: " << c.a.x << std::endl;
	return 0;
}
