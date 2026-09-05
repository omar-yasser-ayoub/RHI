#pragma once

#include "rhi/irhi.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include <webgpu/webgpu_cpp.h>

class WebGPURHI : public IRHI
{
public:
    bool Initialize();
    void Shutdown();

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc);
    rhi::TextureViewHandle CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle texture);
    rhi::ShaderModule CompileShader(rhi::ShaderSource source);

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
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
    std::vector<wgpu::Sampler> m_Samplers;
    std::vector<wgpu::Buffer> m_Buffers;
    std::vector<rhi::BufferDesc> m_BufferMetadata;
    std::vector<wgpu::Texture> m_Textures;
    std::vector<wgpu::TextureView> m_TextureViews;
};
