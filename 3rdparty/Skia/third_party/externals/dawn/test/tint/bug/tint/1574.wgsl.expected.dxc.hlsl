struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer a_u32 : register(u0);
RWByteAddressBuffer a_i32 : register(u1);
groupshared uint b_u32;
groupshared int b_i32;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    uint v = 0u;
    InterlockedExchange(b_u32, 0u, v);
    int v_1 = int(0);
    InterlockedExchange(b_i32, int(0), v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  uint value = 42u;
  uint v_2 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_2);
  uint v_3 = v_2;
  atomic_compare_exchange_result_u32 r1 = {v_3, (v_3 == 0u)};
  uint v_4 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_4);
  uint v_5 = v_4;
  atomic_compare_exchange_result_u32 r2 = {v_5, (v_5 == 0u)};
  uint v_6 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_6);
  uint v_7 = v_6;
  atomic_compare_exchange_result_u32 r3 = {v_7, (v_7 == 0u)};
  int value_1 = int(42);
  int v_8 = int(0);
  a_i32.InterlockedCompareExchange(0u, int(0), value_1, v_8);
  int v_9 = v_8;
  atomic_compare_exchange_result_i32 r1_1 = {v_9, (v_9 == int(0))};
  int v_10 = int(0);
  a_i32.InterlockedCompareExchange(0u, int(0), value_1, v_10);
  int v_11 = v_10;
  atomic_compare_exchange_result_i32 r2_1 = {v_11, (v_11 == int(0))};
  int v_12 = int(0);
  a_i32.InterlockedCompareExchange(0u, int(0), value_1, v_12);
  int v_13 = v_12;
  atomic_compare_exchange_result_i32 r3_1 = {v_13, (v_13 == int(0))};
  uint value_2 = 42u;
  uint v_14 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_14);
  uint v_15 = v_14;
  atomic_compare_exchange_result_u32 r1_2 = {v_15, (v_15 == 0u)};
  uint v_16 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_16);
  uint v_17 = v_16;
  atomic_compare_exchange_result_u32 r2_2 = {v_17, (v_17 == 0u)};
  uint v_18 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_18);
  uint v_19 = v_18;
  atomic_compare_exchange_result_u32 r3_2 = {v_19, (v_19 == 0u)};
  int value_3 = int(42);
  int v_20 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_20);
  int v_21 = v_20;
  atomic_compare_exchange_result_i32 r1_3 = {v_21, (v_21 == int(0))};
  int v_22 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_22);
  int v_23 = v_22;
  atomic_compare_exchange_result_i32 r2_3 = {v_23, (v_23 == int(0))};
  int v_24 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_24);
  int v_25 = v_24;
  atomic_compare_exchange_result_i32 r3_3 = {v_25, (v_25 == int(0))};
}

[numthreads(16, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

