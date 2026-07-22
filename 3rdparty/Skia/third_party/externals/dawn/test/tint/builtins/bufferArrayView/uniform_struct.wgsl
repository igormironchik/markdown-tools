struct S {
  a : vec4f,
  b : vec3f,
  c : vec2f
}

@group(0) @binding(0) var<uniform> v : buffer<128>;

@group(0) @binding(1) var<storage, read_write> out : vec2f;

@compute @workgroup_size(1)
fn main() {
  let p = bufferArrayView<array<S>>(&v, 0, 96);
  out = p[0].c;
}

