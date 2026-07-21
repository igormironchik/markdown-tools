enable chromium_experimental_framebuffer_fetch;
enable f16;

struct Out {
  @location(0)
  x : vec4<f16>,
  @location(2)
  y : vec4<f16>,
  @location(4)
  z : vec4<f16>,
}

@fragment
fn f(@color(1) fbf_1 : vec4<f16>, @color(3) fbf_3 : vec4<f16>) -> Out {
  return Out(fbf_1, vec4<f16>(2.0h), fbf_3);
}
