#pragma once

namespace m {

#if defined M1
#pragma pack(push, 1)
#endif //M1

struct Y;

struct X
{
  char a;
  int y;
#if defined M1
  int z;
#endif // M1

#if defined M3
  char w;
#endif //M3
};


inline int Func(const X& x)
{
#if defined M1
  return x.y + x.z;
#elif defined M3
  return x.y + x.w;
#else
  return x.y + 1;
#endif
}



#if defined M1
#pragma pack(pop)
#endif //M1

struct Y
{
  char a;
  int X;
};

}


int Boo(m::X* x);
