var<workgroup> v : buffer<64>;

fn foo(p : ptr<workgroup, buffer<64>>) {
  let p2 = bufferArrayView<array<vec2f>>(p, 0, 32);
  p2[0].x = 1.0;
}

@compute @workgroup_size(1)
fn main() {
  foo(&v);
}

