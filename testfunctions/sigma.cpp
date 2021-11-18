int sigma_n(int n, int arr[256]) {
#pragma HLS INTERFACE m_axi port=arr
  
  if (n > 128) {
    return -1;
  }


  int sum = 0;
  for (int i = 0; i < n; i ++) {
    sum += i + 1;
  }

  return sum;
}
