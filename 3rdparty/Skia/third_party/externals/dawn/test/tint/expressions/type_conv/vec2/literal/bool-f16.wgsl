// flags:  --hlsl-shader-model 6.2
enable f16;
var<private> u : vec2<f16> = vec2<f16>(vec2<bool>(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
