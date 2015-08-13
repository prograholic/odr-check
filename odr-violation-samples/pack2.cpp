#include <iostream>
#include <cstdint>
#include <string>

#pragma pack(push, 1)

struct C {
	char c;
	uint32_t x;
};

#pragma pack(pop)

void Modify(C* c, size_t pos);

int main() {
	std::string x = "0";
	C c[8] = {0};
	Modify(c, 6);
	std::cout << x << std::endl;

	return 0;
}
