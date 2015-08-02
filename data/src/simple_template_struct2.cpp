
template <typename T, typename Y>
struct simple_template_struct2
{
  T x1;
  int x2;
  Y x3;
};

template <typename T>
struct simple_template_struct2<T, char *>
{
  void* x3;
  int x2;
  T x1;
  T x4;
};



template <typename T>
int Ret(const T& t) {
  return t.x1;
}

int F(simple_template_struct2<int, long>& s) {
  return Ret(s);
}

int F(simple_template_struct2<int, char*>& s) {
  return Ret(s);
}