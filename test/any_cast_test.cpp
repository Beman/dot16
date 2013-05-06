#define USE_BOOST
#ifndef USE_BOOST
# include "adobe_any.hpp"
#else
# include <boost/any.hpp>
  using boost::any;
  using boost::any_cast;
#endif
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>

using std::cout;
using std::endl;

    void foo(const any& x)
    {
       //any_cast<int&>(x) = 42;  // fails to compile

       int y;
       y = any_cast<const int&>(x);  // compiles OK

       y = any_cast<int>(x);         // compiles OK
    }

// example given in any proposal 
bool is_string(const any& operand)
{
  return any_cast<std::string>(&operand) != nullptr;
}

enum move_state {unmoved, moved_to, moved_from};

class movable
{
public:
  move_state state;
  movable() : state(unmoved) {}
  movable(const movable&) : state(unmoved) {}
  movable(movable&& x) : state(moved_to) {x.state = moved_from;}
  movable& operator=(const movable&) {state = unmoved; return *this;}
  movable& operator=(movable&& x) {state = moved_to; x.state = moved_from; return *this;}
};

template <class Os>
Os& operator<<(Os& os, const movable& x)
{
  if (x.state == unmoved)
    os << "unmoved";
  else if (x.state == moved_to)
    os << "moved_to";
  else if (x.state == moved_from)
    os << "moved_from";
  else
    os << "invalid";
  return os;
}

any f_any() {return any(movable());}

any& f_any_ref()
{
  static any x;
  return x;
}

//  example code from proposal

void example()
{
  std::cout << "Test example from proposal" << std::endl;

  any x(5);                                      // x holds int
  assert(any_cast<int>(x) == 5);                 // cast to value
  any_cast<int&>(x) = 10;                        // cast to reference
  assert(any_cast<int>(x) == 10); 

  x = "Meow";                                    // x holds const char*
  assert(std::strcmp(any_cast<const char*>(x),
    "Meow") == 0);
  any_cast<const char*&>(x) = "Harry";
  assert(std::strcmp(any_cast<const char*>(x),
   "Harry") == 0);

  x = std::string("Meow");                       // x holds string
  std::string s, s2("Jane");
  s = std::move(any_cast<std::string&>(x));      // move from any
  assert(s == "Meow");
  any_cast<std::string&>(x) = std::move(s2);     // move to any
  assert(any_cast<const std::string&>(x)
    == "Jane");

  std::string cat("Meow");
  const any y(cat);                              // const y holds string
  assert(any_cast<const std::string&>(y)
    == cat);

  //any_cast<std::string&>(y);                     // error; ill-formed to
                                                 //  cast away const

  assert(is_string(y));
  x = 12345;
  assert(!is_string(x));
}

int main()
{
  movable m1;
  assert(m1.state == unmoved);
  movable m2(m1);
  assert(m1.state == unmoved);
  assert(m2.state == unmoved);
  movable m3(std::move(m2));
  assert(m2.state == moved_from);
  assert(m3.state == moved_to);

  movable m4;
  assert(m4.state == unmoved);
  assert(m1.state == unmoved);
  m4 = m1;
  assert(m4.state == unmoved);
  assert(m1.state == unmoved);
  movable m5;
  assert(m5.state == unmoved);
  m5 = std::move(m4);
  assert(m4.state == moved_from);
  assert(m5.state == moved_to);

  movable m6 = any_cast<movable>(f_any());
  assert(m6.state == unmoved);
  m6 = std::move(any_cast<movable>(f_any()));
  assert(m6.state == moved_to);

  f_any_ref() = movable();
  movable o;
  o = std::move(any_cast<movable&>(f_any_ref()));
  cout << o << endl;
  assert(o.state == moved_to);
  cout << any_cast<movable&>(f_any_ref()) << endl;
  assert(any_cast<movable&>(f_any_ref()).state == moved_from);
 
  //any a;
  //a = std::string("1234567890123456789012345678901234567890bingo");
  //std::string s;
  //s = any_cast<std::string&&>(std::move(a));
  //cout << " s is \"" << s << '"' << endl;
  //if (a.empty())
  //  cout << " a is empty" << endl;
  //else
  //  cout << " a is \"" << any_cast<const std::string&>(a) << '"' << endl;
  any a;
  a = std::string("1234567890123456789012345678901234567890bingo");
  std::string s;
  s = std::move(any_cast<std::string&>(a));
  cout << " s is \"" << s << '"' << endl;
  if (a.empty())
    cout << " a is empty" << endl;
  else
    cout << " a is \"" << any_cast<const std::string&>(a) << '"' << endl;

  any x(5);
  cout << x.type().name() << endl;
  assert(any_cast<int>(x) == 5);
  any_cast<int&>(x) = 10;
  assert(any_cast<int>(x) == 10);
  //any_cast<int*>(x);  // throws bad_any_cast
  any_cast<const int>(x);
  any_cast<const int&>(x);
  //any_cast<const int*>(x);    // throws bad_any_cast

  x = "Meow";
  cout << x.type().name() << endl;
  //any_cast<char>(x);  // throws bad_any_cast
  //any_cast<char&>(x);  // throws bad_any_cast
  //any_cast<char*>(x);  // throws bad_any_cast
  //any_cast<const char>(x);  // throws bad_any_cast
  //any_cast<const char&>(x);  // throws bad_any_cast
  assert(std::strcmp(any_cast<const char*>(x), "Meow") == 0);
  any_cast<const char*&>(x) = "Harry";
  assert(std::strcmp(any_cast<const char*>(x), "Harry") == 0);

  std::string cat("Meow");
  const any y(cat);
  cout << y.type().name() << endl;

  assert(is_string(y));

  assert(any_cast<std::string>(y) == cat);
  //any_cast<std::string&>(y);  // error, conversion loses qualifiers
  //any_cast<std::string*>(y);  // throws bad_any_cast
  assert(any_cast<const std::string>(y) == cat);
  assert(any_cast<const std::string&>(y) == cat);
  //any_cast<const std::string*>(y);  // throws bad_any_cast

  {
  any x(5);
  assert(any_cast<int>(&x) != nullptr);
  assert(any_cast<long>(&x) == nullptr);
  assert(any_cast<int*>(&x) == nullptr);

  int i;
  x = &i;
  assert(any_cast<int*>(&x) != nullptr);

  
  }

  example();

  return 0;
}
