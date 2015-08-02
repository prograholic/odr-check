

template <typename T>
struct no_default;

template <>
struct no_default<int>
{
  int x;
};


void Qux(no_default<int>& x) {
}