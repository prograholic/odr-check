struct A {
	int x;
};

struct B {
	int y;
};

struct C {
	A a;
	B b;
};

void Modify(C& c) {
	c.a.x = 10;
	c.b.y = 20;
}