int top(int trace[258]) {
  int acc = 0;
  for (int i = 0; i < 5; i++) {
    int j = 0;
    while (j < 10) {
      acc += i * j;
      j++;
    }
  }

  if (acc > 100) {
    for (int i = 0; i < 20; i++) {
      acc -= i;
    }
  }

  return acc;
}
