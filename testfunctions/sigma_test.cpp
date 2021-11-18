#include <iostream>

#define ARR_SZ 256

extern int sigma_n(int, int a[ARR_SZ]);

void run_test(int n, int ans, int *test) {
  printf("Running sigma_n(%d, test)...\n", n);
  int out = sigma_n(n, test);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], test[i], ",]"[i==ARR_SZ-1]);
  printf("\n");
  if (out != ans) {
    printf("Expected sigma_n(%d, test) to be %d but got %d.\n", n, ans, out);
    exit(1);
  }
}

int main() {
  std::cout << "Entered main" << std::endl;
  int test[ARR_SZ] = {0};
  
  run_test(5, 15, test);
  run_test(20, 210, test);

  return 0;
}
