static float cond[10] = {0.3, 0.4, 0.8, 0.7, 0.1, 0.6, 0.9, 0.5, 0.2, 1.0};

int top(int trace[256], int n, float if_prob) {
#pragma HLS INTERFACE m_axi port=trace

  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum = 0;
    if (cond[i] <= if_prob) {
      for (int i = 0; i < n; i ++) {
        sum += i + 1;
      }
    }
    else {
      for (int i = 0; i < n; i ++) {
        sum += i + 1;
      }
    }
  }

  return sum;
}
