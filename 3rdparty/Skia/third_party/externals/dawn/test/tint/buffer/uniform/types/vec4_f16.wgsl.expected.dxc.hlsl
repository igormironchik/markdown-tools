
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> x = tint_bitcast_to_f16(u[0u].xy);
  s.Store<vector<float16_t, 4> >(0u, x);
}

