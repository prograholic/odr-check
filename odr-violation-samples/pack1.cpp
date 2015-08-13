#include <cstdint>

struct C {
	char c;
	uint32_t x;
};

void Modify(C* c, size_t pos) {
	c[pos].x += 1;;
}
