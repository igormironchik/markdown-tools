//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 v = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_1 = (v << uint4(0u, 16u, 0u, 16u));
  return uint2((v_1.x | v_1.y), (v_1.z | v_1.w));
}

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, bitcast_81c5f5());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 v = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_1 = (v << uint4(0u, 16u, 0u, 16u));
  return uint2((v_1.x | v_1.y), (v_1.z | v_1.w));
}

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, bitcast_81c5f5());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 v = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_1 = (v << uint4(0u, 16u, 0u, 16u));
  return uint2((v_1.x | v_1.y), (v_1.z | v_1.w));
}

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = bitcast_81c5f5();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

