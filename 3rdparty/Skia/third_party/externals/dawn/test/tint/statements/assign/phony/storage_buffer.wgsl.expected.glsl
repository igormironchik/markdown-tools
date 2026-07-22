#version 310 es


struct S {
  int i;
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  S inner;
} v;
layout(binding = 1, std430)
buffer v1_block_1_ssbo {
  float inner;
} v_1;
layout(binding = 2, std430)
buffer v2_block_1_ssbo {
  uvec3 inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
