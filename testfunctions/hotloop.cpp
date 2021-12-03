int top(int trace[258], int in[128], int out[128], int data) {
#pragma HLS interface bram port=in
#pragma HLS interface bram port=out

  if (data <= 3) {
IF_LOOP: for (int i = 0; i < 96; i++) {
#pragma HLS pipeline off
      out[i] = in[i];
    }
  } else {
ELSE_LOOP: for (int i = 0; i < 64; i++) {
#pragma HLS pipeline off
      out[i] = in[i];
    }
  }

  return 0;
}
