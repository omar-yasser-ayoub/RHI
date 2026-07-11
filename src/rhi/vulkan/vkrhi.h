#pragma once

#include "rhi/irhi.h"
#include <vulkan/vulkan.h>
#include <vector>


class VulkanRHI : public IRHI
{
public:
    bool Initialize();
    void Shutdown();

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data);

    void* MapBuffer(rhi::BufferHandle handle);
    void UnmapBuffer(rhi::BufferHandle handle);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkFilter toVk(rhi::Filter mode);
    VkSamplerMipmapMode toVk(rhi::MipmapFilter mode);
    VkSamplerAddressMode toVk(rhi::AddressMode mode);
    VkCompareOp toVk(rhi::CompareOp mode);
    VkBorderColor toVk(rhi::BorderColor mode);
    VkBufferUsageFlags toVk(rhi::BufferUsage usage);
    VkMemoryPropertyFlags toVk(rhi::MemoryUsage usage);

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    std::vector<VkSampler> m_Samplers;
    std::vector<VkBuffer> m_Buffers;
    std::vector<VkDeviceMemory> m_BufferMemory;
};
