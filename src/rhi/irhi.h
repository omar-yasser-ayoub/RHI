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
    virtual ~IRHI() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc) = 0;
    virtual rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc) = 0;
    virtual rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr) = 0;

    virtual void DestroySampler(rhi::SamplerHandle handle) = 0;
    virtual void DestroyTexture(rhi::TextureHandle handle) = 0;
    virtual void DestroyBuffer(rhi::BufferHandle handle) = 0;

    virtual void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0) = 0;
    virtual void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) = 0;

    const RHICapabilities& GetCapabilities() const
    {
        return m_Capabilities;
    }

protected:
    RHICapabilities m_Capabilities;
};
