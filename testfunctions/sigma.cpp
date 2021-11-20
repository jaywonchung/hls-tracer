int top(int trace[257], int n) {
#pragma HLS INTERFACE m_axi port=trace
  
  if (n > 128) {
    return -1;
  }

  int sum = 0;
  for (int i = 0; i < n; i ++) {
    sum += i + 1;
  }

  return sum;
}
