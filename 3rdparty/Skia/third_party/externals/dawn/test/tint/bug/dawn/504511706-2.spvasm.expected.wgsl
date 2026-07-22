var<private> main_position_Output : vec4<f32>;

struct VertexInput {
  position : f32,
  color : f32,
}

struct VertexOutput {
  position : f32,
  color : f32,
}

fn main_inner(in : VertexInput) -> VertexOutput {
  var out : VertexOutput = VertexOutput();
  return out;
}

fn main_inner_1() {
  main_inner(VertexInput(1.0f, 1.0f));
}

@vertex
fn main() -> @builtin(position) vec4<f32> {
  main_inner_1();
  return main_position_Output;
}
