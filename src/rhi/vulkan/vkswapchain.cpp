#include "rhi/vulkan/vkrhi.h"

rhi::SwapchainHandle VulkanRHI::CreateSwapchain(const rhi::SwapchainDesc& desc)
{
    return {};
}

void VulkanRHI::DestroySwapchain(rhi::SwapchainHandle handle)
{
}

rhi::AcquireResult VulkanRHI::AcquireNextImage(rhi::SwapchainHandle handle)
{
    return {};
}

void VulkanRHI::Present(rhi::SwapchainHandle handle)
{
}
