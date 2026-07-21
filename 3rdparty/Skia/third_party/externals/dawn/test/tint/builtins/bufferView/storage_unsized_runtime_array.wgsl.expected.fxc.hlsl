
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  uint v_2 = ((((v_1 < 4u)) ? (0u) : (0u)) * 1u);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  uint v_4 = (((v_3 - (0u + v_2)) / 4u) - 1u);
  v.Store(((0u + v_2) + (min(uint(int(4)), v_4) * 4u)), 4u);
}

