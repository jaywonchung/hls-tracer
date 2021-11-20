int top(int trace[258], int n) {
#pragma HLS INTERFACE m_axi port=trace

  // if (n > 128) {
  //   return -1;
  // }

  int sum = 0;

  for (int i = 0; i < n; i ++) {
    sum = 0;
    if (i % 2 == 0) {

      int j = 0;
      while (j < i) {
        if (j % 2 == 0) {
          j += 1;
        }
        else {
          j += 1;
        }
      }
      for (int k = 0; k < j + 1; k ++) {
        sum += k + 1;
      }
    }
    else {
      int j = -1;

      do {
        if (j % 2 == 0) {
          j += 1;
        }
        else {
          j += 1;
        }
      } while (j < i);

      for (int k = 0; k < j + 1; k ++) {
        sum += k + 1;
      }
    }
  }
  return sum;
}
