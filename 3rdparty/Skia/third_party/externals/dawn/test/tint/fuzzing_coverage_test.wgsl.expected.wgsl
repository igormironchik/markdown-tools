@group(0u) @binding(0u) var<storage, read_write> out_buf : u32;

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  out_buf = 42u;
}
