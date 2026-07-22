// flags:  --hlsl-shader-model 6.2
enable f16;
var<private> u = vec3<i32>(1i);

@compute @workgroup_size(1)
fn f() {
    let v : vec3<f16> = vec3<f16>(u);
}
