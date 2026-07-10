#pragma once
#include "../irhi.h"
#include <webgpu/webgpu_cpp.h>

class WebGPURHI : public IRHI
{
public:
    bool Initialize();
    void Shutdown();

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data);

private:
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
};
