int top(int trace[258]) {
  int acc[16] = {0,};
  for (int i = 0; i < 5; i++) {
#pragma HLS pipeline off
    for (int j = 0; j < 16; j++) {
#pragma HLS pipeline off
#pragma HLS unroll
      acc[j] += i * j;
    }
  }

  return acc[0] + acc[15];
}
