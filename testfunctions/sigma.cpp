int sigma_n(int n, int arr[256]) {
#pragma HLS INTERACE m_axi port=arr
#pragma HLS INTERFACE s_axilite port=return
  
  if (n > 128) {
    return -1;
  }

  // Write data to DRAM?
  for (int i = 0; i < n; i ++) {
    arr[i] = i + 1;
  }

  // Read data and aggregate to sum
  int sum = 0;
  for (int i = 0; i < n; i ++) {
    sum += arr[i];
  }

  return sum;
}