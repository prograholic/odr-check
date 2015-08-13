#include <string>
#include <boost/noncopyable.hpp>

struct A : private boost::noncopyable{
	A();
	int y;
	const std::string x;
};

void Modify(A& a) {
	a.y += 1;
}
