
int f(int a, int b, int c) {
  return asint((asuint(asint((asuint(a) * asuint(b)))) + asuint(c)));
}

int vec3u(int a) {
  return a;
}

[numthreads(1, 1, 1)]
void main() {
  f(int(1), int(2), int(3));
  int x = int(3);
  vec3u(x);
  vec3u(x);
  vec3u(x);
  vec3u(x);
  int z = int(3);
  int v = int(z);
  int v_1 = int(z);
}

