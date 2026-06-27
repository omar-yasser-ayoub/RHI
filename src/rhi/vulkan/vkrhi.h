#pragma once

#include "../IRHI.h"
#include <vulkan/vulkan.h>

class VulkanRHI : public IRHI
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
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
};
