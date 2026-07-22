#version 310 es

shared uvec4 v[4];
layout(binding = 0, std430)
buffer out_block_1_ssbo {
  uint inner;
} v_1;
void foo() {
  v_1.inner = 64u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
}
