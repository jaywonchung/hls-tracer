#include <iostream>

#define ARR_SZ 256

extern int loop(int, int a[ARR_SZ], float);

void run_test(int n, int ans, int *test, float if_prob) {
  printf("Running loop(%d, test)...\n", n);
  int out = loop(n, test, if_prob);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], test[i], ",]"[i==ARR_SZ-1]);
  printf("\n");
  if (out != ans) {
    printf("Expected loop(%d, test, %f) to be %d but got %d.\n", n, if_prob, ans, out);
    exit(1);
  }
}

int main() {
  printf("Entered main.\n");
  int test[ARR_SZ] = {0};

  for (int tc=0; tc<10; tc++) {
    run_test(5, 15, test, 0.1 * (tc+1));
  }

  return 0;
}
