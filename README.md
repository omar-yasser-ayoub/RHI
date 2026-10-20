This RHI serves as an abstraction layer over Vulkan, DX12 and WebGPU. The purpose of this is mainly to use for my personal work in the future. I want everything I write to be compilable for the web as a target, and also work with vulkan/dx12 directly locally when i want to.

Here are the main features:

- HLSL shader compiled using DXC to SPIR-V (Vulkan) or DXIL (DX12) and also to webgpu through Tint at compile time (HLSL -> SPIR-V -> WGSL).
- Resource tracking with returned handles for a ECS-ish style of storing data.
- Abstractions for allocating buffers, textures, or any other resources
- Dedicated transfer ring buffer for GPU transfer operations.
- Reflection helpers for bind groups/descriptor sets/resources that are used in the shaders.

There's also some limitations with this RHI that I want to address in the future:
- No frame graph: can't efficiently render and apply barriers, currently i just transition in the middle of passes, which isn't allowed by vulkan due to it crashing on mobile GPUs. This is fine for my GPU though, so not a big issue for my use case for now.
- No dedicated write/free operations: nothing guarantees that writing to or destroying resources won't wait for whatever is using them to finish. This is left to the user to implement for now (the main mechanism is just waiting for a GPU idle and calling it a day).
- No bindless support as WebGPU doesn't and I thought it would be a hassle to be able to support this, but WebGPU is adding bindless soon, so maybe I can move to that on all three backends soon.
- No RT, I didn't really care for it initially because WebGPU also doesn't support it. 

This project mainly served as a way for me to understand the semantics of Vulkan and DX12, as I prefer bottom up learning to understand more. Not sure how useful this RHI would be in a production environment, but its only really meant for personal use!

TY for reading! :)
