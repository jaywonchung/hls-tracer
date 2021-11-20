#include <iostream>
#include <cstring>
#include "get_result_json.h"

#define ARR_SZ 258

extern int top(int a[ARR_SZ], int, float);

void run_test(int n, int ans, int *trace, float if_prob) {
  printf("Running hot_loop(%d, trace)...\n", n);
  int out = top(trace, n, if_prob);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], trace[i], ",]"[i==ARR_SZ-1]);
  printf("\n");
  if (out != ans) {
    printf("Expected hot_loop(%d, trace, %f) to be %d but got %d.\n", n, if_prob, ans, out);
    exit(1);
  }

  std::cout << getResultInJson(trace, ARR_SZ).dump() << std::endl;
}

int main() {
  printf("Entered main.\n");
  int trace[ARR_SZ] = {0};

  for (int tc=0; tc<10; tc++) {
    run_test(5, 15, trace, 0.1 * (tc+1));
    memset(trace, 0, ARR_SZ * sizeof(int));
  }

  return 0;
}
