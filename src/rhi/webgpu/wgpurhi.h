#pragma once

#include "rhi/resources/capabilities.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/bindgroup.h"
#include <webgpu/webgpu_cpp.h>

class WebGPURHI
{
public:
    bool Initialize();
    void Shutdown();

    const rhi::Capabilities& GetCapabilities() const
    {
        return m_Capabilities;
    }

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc);
    rhi::TextureViewHandle CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle texture);
    rhi::ShaderModuleHandle CompileShader(rhi::ShaderSource source);
    const rhi::ShaderModule* GetShaderModule(rhi::ShaderModuleHandle handle) const;
    rhi::BindGroupLayoutHandle CreateBindGroupLayout(const rhi::BindGroupLayoutDesc& desc);
    rhi::BindGroupHandle CreateBindGroup(const rhi::BindGroupDesc& desc);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);
    void DestroyTextureView(rhi::TextureViewHandle handle);

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data);

    void* MapBuffer(rhi::BufferHandle handle);
    void UnmapBuffer(rhi::BufferHandle handle);

    static wgpu::FilterMode toWebGPU(rhi::Filter f);
    static wgpu::MipmapFilterMode toWebGPU(rhi::MipmapFilter f);
    static wgpu::AddressMode toWebGPU(rhi::AddressMode f);
    static wgpu::CompareFunction toWebGPU(rhi::CompareOp op);
    static wgpu::BufferUsage toWebGPU(rhi::BufferUsage usage);
    static wgpu::BufferUsage toWebGPU(rhi::MemoryUsage usage);
    static wgpu::TextureDimension toWebGPU(rhi::TextureDimension dim);
    static wgpu::TextureFormat toWebGPU(rhi::TextureFormat usage);
    static uint32_t toWebGPU(rhi::SampleCount count);
    static wgpu::TextureUsage toWebGPU(rhi::TextureUsage usage);


private:
    rhi::Capabilities m_Capabilities;
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
    std::vector<wgpu::Sampler> m_Samplers;
    std::vector<std::uint64_t> m_FreeSamplers;
    std::vector<wgpu::Buffer> m_Buffers;
    std::vector<std::uint64_t> m_FreeBuffers;
    std::vector<wgpu::Texture> m_Textures;
    std::vector<std::uint64_t> m_FreeTextures;
    std::vector<wgpu::TextureView> m_TextureViews;
    std::vector<std::uint64_t> m_FreeTextureViews;
    std::vector<rhi::ShaderModule> m_ShaderModules;
    std::vector<wgpu::BindGroupLayout> m_BindGroupLayouts;
    std::vector<wgpu::BindGroup> m_BindGroups;
};
