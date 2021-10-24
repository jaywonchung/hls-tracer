int sigma_n(int n) {
  int arr[100];
  
  if (n < 100) {
    return -1;
  }

  for (int i = 0; i < n; i ++) {
    arr[i] = i + 1;
  }

  int sum = 0;
  for (int i = 0; i < n; i ++) {
    sum += arr[i];
  }

  return sum;
}