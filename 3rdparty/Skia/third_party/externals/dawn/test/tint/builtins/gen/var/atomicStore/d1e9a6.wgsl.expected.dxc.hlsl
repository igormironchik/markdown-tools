//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_d1e9a6() {
  int arg_1 = int(1);
  int v = int(0);
  sb_rw.InterlockedExchange(0u, arg_1, v);
}

void fragment_main() {
  atomicStore_d1e9a6();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_d1e9a6() {
  int arg_1 = int(1);
  int v = int(0);
  sb_rw.InterlockedExchange(0u, arg_1, v);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_d1e9a6();
}

