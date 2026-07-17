#include "rhi/vulkan/vkrhi.h"

rhi::TextureHandle VulkanRHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

void VulkanRHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void VulkanRHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
