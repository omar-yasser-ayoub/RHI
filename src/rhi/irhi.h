#pragma once
#include <string>
#include "rhi/resources/texture.h"
#include "rhi/resources/sampler.h"
#include "rhi/resources/buffer.h"

struct RHICapabilities {
    std::string deviceName = "N/A";
    bool SupportsMeshShaders = false;
    bool SupportsRayTracing = false;
};

class IRHI
{
public:
    ~IRHI() = default;

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

    const RHICapabilities& GetCapabilities() const
    {
        return m_Capabilities;
    }

protected:
    RHICapabilities m_Capabilities;
};
