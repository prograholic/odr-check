#pragma pack(push, 1)

struct C {
	char c;
	unsigned int x;
};

#pragma pack(pop)

void Modify(C& c) {
	c.x = 0xABCDEF78;
}
