#include <iostream>
#include <cassert>

extern int sigma_n(int, int a[128]);

int main() {
  std::cout << "Entered main" << std::endl;
  int test[128] = {0};
  
  assert(sigma_n(5, test) == 15 && "Sigma_n(5) expected to be 15");
  assert(sigma_n(20, test) == 210 && "Sigma_n(20) expected to be 210");

  return 0;
}