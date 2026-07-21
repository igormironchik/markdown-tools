@group(0) @binding(0) var<storage, read_write> v : buffer;

struct S {
  a : vec4f,
  b : array<u32>,
}

@compute @workgroup_size(1)
fn main() {
  let p = bufferArrayView<S>(&(v), 0, 64);
  p.b[4] = 4;
}
