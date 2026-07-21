// flags:  --hlsl-shader-model 6.2
enable f16;
var<private> u = i32(1i);

@compute @workgroup_size(1)
fn f() {
    let v : f16 = f16(u);
}
