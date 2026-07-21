struct S {
  int i;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
cbuffer cbuffer_v1 : register(b1) {
  uint4 v1[1];
};
cbuffer cbuffer_v2 : register(b2) {
  uint4 v2[1];
};
S v(uint start_byte_offset) {
  S v_1 = {asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)])};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  v(0u);
  asint(u[0u].x);
  asfloat(v1[0u].x);
}

