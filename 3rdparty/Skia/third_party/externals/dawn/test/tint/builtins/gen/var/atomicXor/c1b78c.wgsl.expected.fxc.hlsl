//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicXor_c1b78c() {
  int arg_1 = int(1);
  int v = int(0);
  sb_rw.InterlockedXor(0u, arg_1, v);
  int res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicXor_c1b78c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicXor_c1b78c() {
  int arg_1 = int(1);
  int v = int(0);
  sb_rw.InterlockedXor(0u, arg_1, v);
  int res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicXor_c1b78c()));
}

