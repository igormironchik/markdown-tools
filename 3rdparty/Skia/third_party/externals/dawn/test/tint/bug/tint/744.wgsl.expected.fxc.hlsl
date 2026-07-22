struct main_inputs {
  uint3 global_id : SV_DispatchThreadID;
};


ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};
void main_inner(uint3 global_id) {
  uint2 resultCell = uint2(global_id.y, global_id.x);
  uint dimInner = uniforms[0u].y;
  uint dimOutter = uniforms[1u].y;
  uint result = 0u;
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    uint i = 0u;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      if ((i < dimInner)) {
      } else {
        break;
      }
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      uint v = 0u;
      firstMatrix.GetDimensions(v);
      uint v_1 = 0u;
      secondMatrix.GetDimensions(v_1);
      result = (result + (firstMatrix.Load((0u + (min(a, ((v / 4u) - 1u)) * 4u))) * secondMatrix.Load((0u + (min(b, ((v_1 / 4u) - 1u)) * 4u)))));
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        i = (i + 1u);
      }
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  uint v_2 = 0u;
  resultMatrix.GetDimensions(v_2);
  resultMatrix.Store((0u + (min(index, ((v_2 / 4u) - 1u)) * 4u)), result);
}

[numthreads(2, 2, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.global_id);
}

