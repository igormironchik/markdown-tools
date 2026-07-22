struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint v[16];
void foo() {
  uint v_1 = ((min(uint(int(0)), ((32u / 8u) - 1u)) * 8u) / 4u);
  v[v_1] = asuint(1.0f);
}

void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 16u)) {
        break;
      }
      v[((v_3 * 4u) / 4u)] = 0u;
      {
        v_2 = (v_3 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

