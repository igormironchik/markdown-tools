@group(0) @binding(0) var<storage, read> v : buffer;

@group(0) @binding(1) var<storage, read_write> out : vec4u;

@fragment
fn main() {
  let offset = 16u;
  let p = bufferArrayView<array<vec4u>>(&v, offset, 16);
  out = p[0];
}

