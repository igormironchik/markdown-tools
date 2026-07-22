
RWByteAddressBuffer out_buf : register(u0);
[numthreads(1, 1, 1)]
void main() {
  out_buf.Store(0u, 42u);
}

