//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 v = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_1 = (v << uint2(0u, 16u));
  return asint((v_1.x | v_1.y));
}

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(bitcast_6ac6f9()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 v = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_1 = (v << uint2(0u, 16u));
  return asint((v_1.x | v_1.y));
}

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(bitcast_6ac6f9()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 v = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_1 = (v << uint2(0u, 16u));
  return asint((v_1.x | v_1.y));
}

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = bitcast_6ac6f9();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

