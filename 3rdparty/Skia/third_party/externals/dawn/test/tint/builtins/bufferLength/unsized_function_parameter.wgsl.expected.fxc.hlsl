
RWByteAddressBuffer v : register(u0);
RWByteAddressBuffer v_1 : register(u1);
void foo(uint tint_array_length) {
  v_1.Store(0u, tint_array_length);
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2 = 0u;
  v.GetDimensions(v_2);
  foo(v_2);
}

