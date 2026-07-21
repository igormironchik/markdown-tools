# Framebuffer Fetch (experimental)

This extension enables support for the
[`chromium_experimental_framebuffer_fetch`](../../tint/extensions/chromium_experimental_framebuffer_fetch.md)
WGSL extension.

The extension is experimental and might change for example to gain new validation rules
(with extension struct) in the future.

It is available on tiler Metal GPUs and Vulkan with the `InputAttachment` capability.

## Validation

In `Device::CreateRenderPipeline` or `Device::CreateRenderPipelineAsync`:
 - For each `@color(N) in : T` fragment in:
   - color target N must exist
   - color target N's format must match T in both component count and base type

## WGSL

### Validation

* Requires `enable chromium_experimental_framebuffer_fetch`
* Requires fragment shader
* Requires T to be `i32`, `u32`, `f32`, `f16` or `vec{2,3,4}{i,u,f,h}`.
* Requires `N` to be unique per `color`
* The `N` value must be in the range of `[0..7]`

### Metal

* `@color(N)` entries translate directly to `[[color(N)]]` annotations on the input data.

### Vulkan

* Requires a `group`, `binding` value provided for each `color` `N` value from Host
* Add the `OpCapability InputAttachment`
* For each `color(N)` entry
  * Create an `OpTypeImage %type SubpassData 0 0 0 2 Unknown` where `%type` is `f32`, `i32` or `u32`. For `f16`,  the `%type` will be `f32` and the `OpImageRead` will be decorated with `RelaxedPrecision`.
  * Create a `OpTypePointer UniformConstant %image`
  * Create a `OpVariable %ptr_uniform_image UniformConstant`
  * Decorate the var with `OpDecorate %inputVar InputAttachmentIndex N`

  * We will then replace the value of the input variable with the value read from the attachment
    * `OpImageRead %v4float %img %zero` where `%zero` is `OpConstantComposite %v2int %int_0 %int_0`
    * For `f16`, the result of the `RelaxedPrecision`-decorated `OpImageRead` is converted to `f16`.

* For Dawn, this would probably map to creating the image in ShaderIO and then calling the
  `image_read` intrinsic for SPIR-V
