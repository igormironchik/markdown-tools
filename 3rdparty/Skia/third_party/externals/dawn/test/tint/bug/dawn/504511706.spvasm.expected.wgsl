struct str_a {
  x : u32,
  y : u32,
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var v : str_a;
  v = str_a(1u, 2u);
}
