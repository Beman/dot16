#ifdef USE_ADOBE
# include "adobe_any.hpp"
  using adobe::any;
  using adobe::any_cast;
#else
# include <boost/any.hpp>
  using boost::any;
  using boost::any_cast;
#endif
#include <cassert>
#include <iostream>

#define MOVABLE_LOG(MSG) std::cout << MSG << std::endl

class movable
{
public:
  movable() {MOVABLE_LOG("  default ctor");}
  movable(const movable&) {MOVABLE_LOG("  copy ctor");}
  movable& operator=(const movable&) {MOVABLE_LOG("  copy assign"); return *this;}
#ifndef NO_MOVE_OPERATORS
  movable(movable&& x) {MOVABLE_LOG("  move ctor");}
  movable& operator=(movable&& x) {MOVABLE_LOG("  move assign"); return *this;}
#endif
};

int main()
{
  std::cout << "create variables uses in subsequent tests" << std::endl;
  movable m;

  std::cout << "copy construct any" << std::endl;
  any x1(m);

  std::cout << "move construct any" << std::endl;
  any x2(std::move(m));

  std::cout << "copy assign to any" << std::endl;
  x1 = m;

  std::cout << "move assign to any" << std::endl;
  x1 = std::move(m);

  std::cout << "copy assign to any contents" << std::endl;
  any_cast<movable&>(x1) = m;
  
  std::cout << "move assign to any contents" << std::endl;
  any_cast<movable&>(x1) = std::move(m);

  std::cout << "copy assign from any contents by value" << std::endl;
  m = any_cast<movable>(x1);

  std::cout << "copy assign from any contents by reference" << std::endl;
  m = any_cast<movable&>(x1);

  std::cout << "copy assign from any contents by const reference" << std::endl;
  m = any_cast<const movable&>(x1);
  
  std::cout << "move assign from any contents by reference" << std::endl;
  m = std::move(any_cast<movable&>(x1));

#ifndef SKIP_ANYCAST_REF_REF
  std::cout << "move assign from any contents by rvalue reference" << std::endl;
  m = any_cast<movable&&>(x1);
#endif

}
