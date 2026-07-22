//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStoreMax_707bde() {
  uint2 arg_1 = (1u).xx;
  uint2 v = arg_1;
  uint64_t v_1 = uint64_t(v.x);
  uint64_t v_2 = uint64_t(v.y);
  sb_rw.InterlockedMax64(0u, ((v_2 << uint64_t(32u)) | v_1));
}

void fragment_main() {
  atomicStoreMax_707bde();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStoreMax_707bde() {
  uint2 arg_1 = (1u).xx;
  uint2 v = arg_1;
  uint64_t v_1 = uint64_t(v.x);
  uint64_t v_2 = uint64_t(v.y);
  sb_rw.InterlockedMax64(0u, ((v_2 << uint64_t(32u)) | v_1));
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStoreMax_707bde();
}

