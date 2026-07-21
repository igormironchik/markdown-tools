// flags: --ycbcr-bindings=0,0

@group(0) @binding(0) var t_f : texture_external;

@compute @workgroup_size(1)
fn main() {
  var vals = textureLoad(t_f, vec2i(0));
}
