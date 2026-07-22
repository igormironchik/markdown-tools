// flags:  --hlsl-shader-model 6.2
enable f16;
var<private> u : f16 = f16(bool(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
