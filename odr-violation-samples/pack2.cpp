#include <iostream>

//#pragma pack(push, 2)

struct C {
	char c;
	unsigned int x;
};

//#pragma pack(pop)

void Modify(C& c);

int main() {
	C c = {0, 0};
	Modify(c);
	std::cout << std::hex << c.x << std::endl;

	return 0;
}
