#include <iostream>
#include "get_result_json.h"

#define ARR_SZ 258

extern int top(int trace[ARR_SZ]);

void run_test(int ans, int *trace) {
  printf("%s\n", "Running call(trace)...");
  int out = top(trace);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], trace[i], ",]"[i==ARR_SZ-1]);
  printf("\n");
  if (out != ans) {
    printf("Expected call(trace) to be %d but got %d.\n", ans, out);
    exit(1);
  }

  json output = getResultInJson(trace, ARR_SZ, "trace.json");
//   std::cout << output.dump() << std::endl;
}

int main() {
  int trace[ARR_SZ] = {0};
  run_test(110, trace);
}
