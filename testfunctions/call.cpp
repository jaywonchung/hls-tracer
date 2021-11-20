// Working example of using sub-functions.
// Basically, you need to add `int *trace` to the definition
// of all sub-functions and `trace` to all call sites.

int abs(int *trace, int data) {
  if (data >= 0) {
    return data;
  } else {
    return -data;
  }
}

int top(int trace[256]) {
  int acc = 0;
  for (int i=0; i<=20; i++) {
    acc += abs(trace, i-10);
  }

  return acc;
}
