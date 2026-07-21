@group(0) @binding(0) var<storage, read> v : buffer;

@group(0) @binding(1) var<storage, read_write> out : vec4u;

@fragment
fn main() {
  let size = 16u;
  let p = bufferArrayView<array<vec4u>>(&(v), 0, size);
  out = p[0];
}
