#include <iostream>
#include <string>
#include <boost/noncopyable.hpp>

struct A : private boost::noncopyable{
	A() : x("0") {
	}
	const std::string x;
	int y;
};

void Modify(A& a);

int main() {
	A a;
	Modify(a);
	std::cout << "a: " << a.x << std::endl;
	return 0;
}
