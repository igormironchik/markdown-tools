
RWByteAddressBuffer a : register(u0);
void main() {
  uint v = 0u;
  a.GetDimensions(v);
  uint v_1 = ((v / 8u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 8u);
  uint64_t v_3 = uint64_t((0u).xx.x);
  uint64_t v_4 = uint64_t((0u).xx.y);
  a.InterlockedMax64((0u + v_2), ((v_4 << uint64_t(32u)) | v_3));
}

