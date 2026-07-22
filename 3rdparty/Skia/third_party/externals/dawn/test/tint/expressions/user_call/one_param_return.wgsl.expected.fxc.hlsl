
int c(int z) {
  int a = asint((asuint(int(1)) + asuint(z)));
  a = asint((asuint(a) + asuint(int(2))));
  return a;
}

[numthreads(1, 1, 1)]
void b() {
  int b_1 = c(int(2));
  int v = b_1;
  b_1 = asint((asuint(v) + asuint(c(int(3)))));
}

