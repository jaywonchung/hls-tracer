int top(int trace[258], int data) {
  int acc[128];

  if (data <= 3) {
IF_LOOP: for (int i = 0; i < 96; i++) {
#pragma HLS pipeline off
      acc[i] = i;
    }
  } else {
ELSE_LOOP: for (int i = 0; i < 64; i++) {
#pragma HLS pipeline off
      acc[i] = i;
    }
  }

  return acc[0] + acc[63] + acc[95];
}
