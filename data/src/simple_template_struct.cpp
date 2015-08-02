
template <typename T>
struct simple_template_struct
{
  T x1;
  int x2;
};

template <>
struct simple_template_struct<char *>
{
  void* x3;
  int x2;
};

typedef simple_template_struct<int> simple_template_struct_int;



template <typename T>
int Ret(const T& t) {
  return t.x2;
}

int Foo(const simple_template_struct_int& s) {
  return Ret(s);
}

int Bar(const simple_template_struct<long>& s) {
  return Ret(s);
}

int Baz(const simple_template_struct<char *>& s) {
  return 0;
}
