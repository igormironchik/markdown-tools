
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void f() {
  uint2 a = uint2(1073757184u, 3288351232u);
  vector<float16_t, 4> b = tint_bitcast_to_f16(a);
}

