@group(0) @binding(0) var<storage, read_write> v : buffer;

fn foo(p : ptr<storage, buffer, read_write>) {
  let p2 = bufferArrayView<array<vec2f>>(p, 0, 32);
  p2[0].y = 3.0;
}

@compute @workgroup_size(1)
fn main() {
  foo(&(v));
}
