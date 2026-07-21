#version 310 es


struct S {
  int i;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std140)
uniform v1_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(binding = 2, std140)
uniform v2_block_1_ubo {
  uvec4 inner[1];
} v_2;
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  return S(int(v_4[((start_byte_offset & 15u) >> 2u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_3(0u);
  uvec4 v_5 = v.inner[0u];
  int v_6 = int(v_5.x);
  uvec4 v_7 = v_1.inner[0u];
  uintBitsToFloat(v_7.x);
  uvec4 v_8 = v_2.inner[0u];
}
