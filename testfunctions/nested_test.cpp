#include <iostream>
#include <cstring>
#include "get_result_json.h"

#define ARR_SZ 258

extern int top(int a[ARR_SZ]);

void run_test(int *trace) {
  printf("%s\n", "Running nested()...");
  int out = top(trace);
  printf("%s\n", "Function successfully returned. Content of trace array:");
  for (int i = 0; i < ARR_SZ; i++)
    printf("%c%d%c", " ["[i==0], trace[i], ",]"[i==ARR_SZ-1]);
  printf("\n");

  json output = getResultInJson(trace, ARR_SZ, "trace.json");

  std::cout << output.dump() << std::endl;
}

int main() {
  printf("Entered main.\n");
  int trace[ARR_SZ] = {0};

  run_test(trace);

  return 0;
}
