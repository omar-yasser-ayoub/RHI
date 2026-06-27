#pragma once
#include "../irhi.h"
#include <webgpu/webgpu_cpp.h>

class WebGPURHI : public IRHI
{
public:
    bool Initialize() override;
    void Shutdown() override;

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc) override;
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc) override;
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr) override;

    void DestroySampler(rhi::SamplerHandle handle) override;
    void DestroyTexture(rhi::TextureHandle handle) override;
    void DestroyBuffer(rhi::BufferHandle handle) override;

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0) override;
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) override;

private:
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
};
