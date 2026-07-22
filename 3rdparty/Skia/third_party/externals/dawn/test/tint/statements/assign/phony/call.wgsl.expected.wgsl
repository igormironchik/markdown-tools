fn f(a : i32, b : i32, c : i32) -> i32 {
  return ((a * b) + c);
}

fn vec3u(a : i32) -> i32 {
  return a;
}

@compute @workgroup_size(1)
fn main() {
  _ = f(1, 2, 3);
  {
    var x : i32 = 3;
    vec3u(x);
    _ = vec3u(x);
    let y = &(x);
    vec3u(*(y));
    _ = vec3u(*(y));
  }
  {
    var z : i32 = 3;
    {
      _ = i32(z);
    }
    let zp = &(z);
    {
      _ = i32(*(zp));
    }
  }
}
