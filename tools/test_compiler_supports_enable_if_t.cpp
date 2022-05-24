#include <type_traits>

struct A {
   void method() {}
};

struct B: A {};

template<typename T>
struct C {
   template<typename U = T>
   std::enable_if_t<std::is_base_of<A, U>::value>
   method_for_A_structs(U u)
   {
      u.method();
   }
};

int main()
{
   B b;
   C<B> c;
   c.method_for_A_structs(b);
}
