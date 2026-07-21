# Transient Attachments

Transient attachments allow render pass operations to stay in tile memory,
avoiding VRAM traffic and potentially avoiding VRAM allocation for the textures.

Presence of the `transient-attachments` feature on an Adapter is a hint
indicating that the `TransientAttachment` usage will be used by the backend. If
it's not present, the usage is allowed, but it is a no-op.
Enabling this feature on a Device doesn't do anything.

Example Usage:

```cpp
wgpu::TextureDescriptor desc;
desc.format = wgpu::TextureFormat::RGBA8Unorm;
desc.size = {1, 1, 1};
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;

auto transientTexture = device.CreateTexture(&desc);

// Can now create views from the texture to serve as transient attachments, e.g.
// as color attachments in a render pipeline.
```

Textures with the `TransientAttachment` usage:

- Must have exactly the usages `RenderAttachment | TransientAttachment`.
- May not remove any usages when creating views.
- May not be used with `load`/`store` in render passes.
- May not be used as resolve attachments.
