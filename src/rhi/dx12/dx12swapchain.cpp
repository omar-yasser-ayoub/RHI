#include "rhi/dx12/dx12rhi.h"

rhi::SwapchainHandle DX12RHI::CreateSwapchain(const rhi::SwapchainDesc& desc)
{
    return {};
}

void DX12RHI::DestroySwapchain(rhi::SwapchainHandle handle)
{
}

rhi::AcquireResult DX12RHI::AcquireNextImage(rhi::SwapchainHandle handle)
{
    return {};
}

void DX12RHI::Present(rhi::SwapchainHandle handle)
{
}
