#version 310 es

int f(int a, int b, int c) {
  uint v = uint(a);
  uint v_1 = uint(int((v * uint(b))));
  return int((v_1 + uint(c)));
}
int vec3u(int a) {
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1, 2, 3);
  int x = 3;
  vec3u(x);
  vec3u(x);
  vec3u(x);
  vec3u(x);
  int z = 3;
  int v_2 = int(z);
  int v_3 = int(z);
}
