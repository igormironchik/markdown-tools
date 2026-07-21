struct S {
  int i;
};


RWByteAddressBuffer s : register(u0);
ByteAddressBuffer v1 : register(t1);
ByteAddressBuffer v2 : register(t2);
S v(uint offset) {
  S v_1 = {asint(s.Load((offset + 0u)))};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  v(0u);
  asint(s.Load(0u));
  asfloat(v1.Load(0u));
  v2.Load3(0u);
  v2.Load(0u);
}

