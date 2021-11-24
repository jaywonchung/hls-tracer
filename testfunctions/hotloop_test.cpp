#include <iostream>
#include <cstring>
#include "get_result_json.h"

#define ARR_SZ 258

extern int top(int arr[ARR_SZ], int n);

void run_test(int n, int ans, int *trace) {
  printf("Running hotloop(trace, %d)...\n", n);
  int out = top(trace, n);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], trace[i], ",]"[i==ARR_SZ-1]);
  printf("\n");
  if (out != ans) {
    printf("Expected hotloop(trace, %d) to be %d but got %d.\n", n, ans, out);
  }

  std::string filename = "trace-" + std::to_string(n) + ".json";
  json output = getResultInJson(trace, ARR_SZ, filename);

  std::cout << output.dump() << std::endl;
}

int main() {
  std::cout << "Entered main" << std::endl;
  int trace[ARR_SZ] = {0};

  /* run_test(0, 15, trace); */
  /* memset(trace, 0, ARR_SZ * sizeof(int)); */

  /* run_test(1, 15, trace); */
  /* memset(trace, 0, ARR_SZ * sizeof(int)); */

  /* run_test(2, 15, trace); */
  /* memset(trace, 0, ARR_SZ * sizeof(int)); */

  run_test(3, 15, trace);
  memset(trace, 0, ARR_SZ * sizeof(int));

  run_test(4, 15, trace);
  memset(trace, 0, ARR_SZ * sizeof(int));

  /* run_test(5, 15, trace); */
  /* memset(trace, 0, ARR_SZ * sizeof(int)); */

  /* run_test(6, 15, trace); */
  /* memset(trace, 0, ARR_SZ * sizeof(int)); */

  return 0;
}
