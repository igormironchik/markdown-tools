
cbuffer cbuffer_v : register(b0) {
  uint4 v[8];
};
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  uint v_2 = (32u + (min(uint(int(0)), ((96u / 48u) - 1u)) * 48u));
  uint4 v_3 = v[(((v_2 / 16u) * 16u) / 16u)];
  v_1.Store2(0u, asuint(asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy))));
}

