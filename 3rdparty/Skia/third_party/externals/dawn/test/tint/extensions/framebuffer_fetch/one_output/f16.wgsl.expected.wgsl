enable chromium_experimental_framebuffer_fetch;
enable f16;

@fragment
fn f(@color(0) fbf : vec4<f16>) -> @location(0) vec4<f16> {
  return fbf;
}
