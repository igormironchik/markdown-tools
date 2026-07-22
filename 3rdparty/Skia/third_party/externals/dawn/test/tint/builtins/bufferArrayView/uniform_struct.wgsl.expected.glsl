#version 310 es

layout(binding = 0, std140)
uniform v_block_1_ubo {
  uvec4 inner[8];
} v_1;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  vec2 inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = (32u + (min(uint(0), ((96u / 48u) - 1u)) * 48u));
  uvec4 v_4 = v_1.inner[(v_3 / 16u)];
  v_2.inner = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2((((v_3 & 15u) >> 2u) == 2u))));
}
