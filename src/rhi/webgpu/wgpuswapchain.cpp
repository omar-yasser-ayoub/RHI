#include "rhi/webgpu/wgpurhi.h"

rhi::SwapchainHandle WebGPURHI::CreateSwapchain(const rhi::SwapchainDesc& desc)
{
    return {};
}

void WebGPURHI::DestroySwapchain(rhi::SwapchainHandle handle)
{
}

rhi::AcquireResult WebGPURHI::AcquireNextImage(rhi::SwapchainHandle handle)
{
    return {};
}

void WebGPURHI::Present(rhi::SwapchainHandle handle)
{
}
