
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  v.Store((0u + (min(uint(int(0)), (((128u - 0u) / 4u) - 1u)) * 4u)), 2u);
}

