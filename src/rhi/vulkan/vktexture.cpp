#include "rhi/resources/texture.h"
#include "rhi/vulkan/vkrhi.h"

rhi::TextureHandle VulkanRHI::CreateTexture(const rhi::TextureDesc& desc) {

    if (desc.width <= 0 || desc.height <= 0 || desc.depth <= 0 || desc.mipLevels <= 0 || desc.arrayLayers <= 0) {
        printf("[ERROR] Texture dimensions must be greater than 0\n");
        return rhi::TextureHandle{};
    }

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = toVk(desc.dimension);
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = desc.depth;
    imageInfo.mipLevels = desc.mipLevels;
    imageInfo.arrayLayers = desc.arrayLayers;
    imageInfo.format = toVk(desc.format);
    imageInfo.samples = toVk(desc.sampleCount);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = toVk(desc.usage);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.flags = 0;

    VkImage image = VK_NULL_HANDLE;

    VkResult result = vkCreateImage(m_Device, &imageInfo, nullptr, &image);

    if (result != VK_SUCCESS) {
        printf("[ERROR] Failed to create texture image\n");
        return rhi::TextureHandle{};
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(m_Device, image, &memReqs);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.allocationSize = memReqs.size;

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        printf("[ERROR] Failed to allocate texture memory\n");
        vkDestroyImage(m_Device, image, nullptr);
        return rhi::TextureHandle{};
    }

    if (vkBindImageMemory(m_Device, image, memory, 0) != VK_SUCCESS) {
        printf("[ERROR] Failed to bind texture memory\n");
        vkDestroyImage(m_Device, image, nullptr);
        vkFreeMemory(m_Device, memory, nullptr);
        return rhi::TextureHandle{};
    }

    m_Textures.push_back(image);
    m_TextureMemory.push_back(memory);
    rhi::TextureHandle handle;
    handle.id = m_Textures.size() - 1;
    handle.desc = desc;
    return handle;
}

void VulkanRHI::DestroyTexture(rhi::TextureHandle handle) {
    if (handle.id >= m_Textures.size() || handle.id >= m_TextureMemory.size()) {
        return;
    }

    VkImage image = m_Textures[handle.id];
    if (image != VK_NULL_HANDLE) {
        vkDestroyImage(m_Device, image, nullptr);
        m_Textures[handle.id] = VK_NULL_HANDLE;
    }

    VkDeviceMemory memory = m_TextureMemory[handle.id];
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_Device, memory, nullptr);
        m_TextureMemory[handle.id] = VK_NULL_HANDLE;
    }
}

void VulkanRHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    // TODO: Implement texture write
}

rhi::TextureViewHandle VulkanRHI::CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle texture) {
    if (texture.id >= m_Textures.size() || texture.id >= m_TextureMemory.size()) {
        printf("[ERROR] Texture handle out of bounds\n");
        return rhi::TextureViewHandle{};
    }

    if (m_Textures[texture.id] == VK_NULL_HANDLE) {
        printf("[ERROR] Texture handle is null\n");
        return rhi::TextureViewHandle{};
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Textures[texture.id];

    VkImageViewType viewType = toVk(desc.dimension, desc.arrayLayers);
    if (viewType == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
        printf("[ERROR] Invalid texture dimension\n");
        return rhi::TextureViewHandle{};
    }
    viewInfo.viewType = viewType;
    viewInfo.format = toVk(desc.format);
    viewInfo.subresourceRange.aspectMask = toVk(desc.aspect);
    viewInfo.subresourceRange.baseMipLevel = desc.baseMipLevel;
    viewInfo.subresourceRange.levelCount = desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = desc.baseArrayLayer;
    viewInfo.subresourceRange.layerCount = desc.arrayLayers;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageView view = VK_NULL_HANDLE;
    VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &view);
    if (result != VK_SUCCESS) {
        printf("[ERROR] Failed to create texture image view\n");
        return rhi::TextureViewHandle{};
    }

    m_TextureViews.push_back(view);

    rhi::TextureViewHandle handle;
    handle.id = m_TextureViews.size() - 1;
    handle.desc = desc;
    return handle;
}

void VulkanRHI::DestroyTextureView(rhi::TextureViewHandle handle) {
    if (handle.id >= m_TextureViews.size()) {
        return;
    }
    VkImageView view = m_TextureViews[handle.id];
    if (view != VK_NULL_HANDLE) {
        vkDestroyImageView(m_Device, view, nullptr);
        m_TextureViews[handle.id] = VK_NULL_HANDLE;
    }
    return;
}
