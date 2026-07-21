# `OpaqueYCbCrAndroid` for external textures.

[Google-internal design doc](https://docs.google.com/document/d/1sD4Dg1-ZDuPzdDBE6UOtsZuZAr0xOMk6-P8X98wQcfw)

## Context

WebGPU is used on the Web and in native to do GPU-accelerated video processing.
Video streams are usually coming from some system component (HW video decoder, camera hardware) and encoded in some YUV format (as a basic form of compression because the human eye is more sensitive to variation in luminance than chrominance).
For performance and power-efficiency reasons, it is important that Dawn/WebGPU (and the applications using it) be able to directly sample YUV data in the shader instead of first decompressing to RGB as both the decompression and the sampling of expanded RGB data cause additional memory traffic than necessary.

For this purpose, the WebGPU API added the [GPUExternalTexture](https://gpuweb.github.io/gpuweb/#gpuexternaltexture) concept that can be created from a HTML \<video\> element, a WebCodec VideoFrame, etc.
In WGSL shaders, a [sampling builtin](https://gpuweb.github.io/gpuweb/wgsl/#textureSampleBaseClampToEdge) is used that is polyfilled in Tint to [generate](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/tint/lang/core/ir/transform/multiplanar_external_texture.h) all the multiplane sampling, YUV to RGB and colorspace conversion, and additional computations needed to sample 0-copy.
On systems where the Y and UV planes of textures can be sampled separately, the generic Tint polyfill works well, but not on Android because videos are AHardwareBuffers with opaque YCbCr formats.

On Android with AHB with [YCbCr formats](https://developer.android.com/ndk/reference/group/a-hardware-buffer#group___a_hardware_buffer_1ga94397844440c1c27b5b454d56ba5ff38) (which is what's given by camera hardware and video decoders), the Vulkan API requires usage of static samplers and more.
Dawn has a large API in [y_cb_cr_vulkan_samplers.md](./y_cb_cr_vulkan_samplers.md) that's  mostly passthrough to Vulkan but it is quite inflexible and difficult to use (future Vulkan extension could improve this but would take time to be available everywhere).

## Creating `ExternalTexture` from `OpaqueYCbCrAndroid`.

This extension `OpaqueYCbCrAndroidForExternalTexture` allows creating an `ExternalTexture` from imported YCbCr AHB, and have sampling "just work".
Internally Dawn recognizes that a pipeline will use an `ExternalTexture` and uses the same mechanisms as [y_cb_cr_vulkan_samplers.md](./y_cb_cr_vulkan_samplers.md) to do 0-copy YUV sampling, recreating `BindGroupLayouts` with static samplers and recompiling the underlying `VkPipeline`.
With this functionality it is possible to 0-copy sample YCbCr AHBs with code like:

```cpp
// Import the AHB in Dawn, wrap it in an STM first (the object doing access control
// and vending indivial wgpu::Texture views of the AHB), then in a wgpu::Texture and
// finally wrap the wgpu::Texture in the wgpu::ExternalTexture object (that is used
// to represent all the other ways to do YUV 0-copy imports).
wgpu::SharedTextureMemory stm = device.CreateSharedTextureMemory(&descFromAHB);
// descForTexture needs to be compatible with Dawn's reflection in stm.GetProperties()
// of what's allowed with that STM (size, usage, format).
wgpu::Texture tex = stm.CreateTexture(&descForTexture);
wgpu::ExternalTexture et = device.CreateExternalTexture(&descWithPlane0Only);

// Start Dawn's access of the AHB, the application needs to pass in the fences required
// to ensure proper ordering of (GPU-timeline) operations on the AHB.
stm.BeginAccess(&beginAccessWithFencesEtc);

// Render, sampling `et` in pipeline to do 0-copy YUV sampling.

// End Dawn's access of the AHB. Dawn will return fences that must be used to
// to synchronize, on the GPU-timeline, the subsequent accesses on the AHB.
stm.EndAccess(&endAccessDesc);

// Destroy stm, tex, et if they are used for a single frame, or keep them around
// if we know that video decoders reuse AHBs internally.
```

## Validation rules

 - This feature allows using `wgpu::TextureFormat::OpaqueYCbCrAndroid` but does not enable any use of them except noted below.
 - `Texture::CreateView` is allowed on `OpaqueYCbCrAndroid` (without a `YCbCrVkDescriptor`).
 - These views can be used in `ExternalTextureDescriptor::plane0` as the single plane specified (views with a `YCbCrVkDescriptor` are rejected).
 - These views *cannot* be used in `BindGroups` even with YCbCr static samplers.
