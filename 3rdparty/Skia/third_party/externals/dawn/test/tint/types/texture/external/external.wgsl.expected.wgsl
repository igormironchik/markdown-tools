@group(0) @binding(0) var t_f : texture_external;

@group(1) @binding(0) var<storage, read_write> out : vec4f;

@compute @workgroup_size(1)
fn main() {
  out = textureLoad(t_f, vec2i(0));
}
