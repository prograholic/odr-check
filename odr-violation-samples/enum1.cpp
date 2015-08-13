enum Foo {
	e1,
	eCount
};
const char xxx [] = "can you see me?!!";
const char* msg[eCount] =  {
	"e1",
};
const char* canYouSeeMe = xxx;
const char* GetMsg(Foo x) {
	return msg[x];
}
