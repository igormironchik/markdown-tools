override v0: bool = true;

fn foo() {
  loop {
    continuing {
      break if v0;
    }
  }
}

@compute @workgroup_size(1)
fn main() {}
