struct S {
    i : i32,
};

@group(0) @binding(0) var<uniform> u : S;
@group(0) @binding(1) var<uniform> v1 : f32;
@group(0) @binding(2) var<uniform> v2 : vec3u;

@compute @workgroup_size(1)
fn main() {
    _ = u;
    _ = u.i;
    _ = v1;
    _ = v2;
    _ = v2.x;
}
