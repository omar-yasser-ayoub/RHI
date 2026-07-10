#pragma once

#include "../IRHI.h"
#include <vulkan/vulkan.h>
#include <vector>


class VulkanRHI : public IRHI
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

    VkFilter toVk(rhi::Filter mode);
    VkSamplerMipmapMode toVk(rhi::MipmapFilter mode);
    VkSamplerAddressMode toVk(rhi::AddressMode mode);
    VkCompareOp toVk(rhi::CompareOp mode);
    VkBorderColor toVk(rhi::BorderColor mode);

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    std::vector<VkSampler> m_Samplers;
};
