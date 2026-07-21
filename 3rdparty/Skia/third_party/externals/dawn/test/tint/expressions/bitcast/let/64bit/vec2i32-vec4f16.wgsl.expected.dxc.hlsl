
vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void f() {
  int2 a = int2(int(1073757184), int(-1006616064));
  vector<float16_t, 4> b = tint_bitcast_to_f16(a);
}

