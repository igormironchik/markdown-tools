// flags:  --hlsl-shader-model 6.2
enable f16;
var<private> v = vec2(0.0h, 1.0h);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
