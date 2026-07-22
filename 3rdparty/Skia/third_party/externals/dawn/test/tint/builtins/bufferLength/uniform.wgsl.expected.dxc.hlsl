
cbuffer cbuffer_v : register(b0) {
  uint4 v[4];
};
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  v_1.Store(0u, 64u);
}

