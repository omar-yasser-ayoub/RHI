#include <iostream>

#ifdef RHI_VULKAN
    #include "rhi/vulkan/VkRHI.h"
#elif defined(RHI_DX12)
    #include "rhi/dx12/DX12RHI.h"
#elif defined(RHI_WEBGPU)
    #include "rhi/webgpu/WGPURHI.h"
#endif

int main()
{
#ifdef RHI_VULKAN
    VulkanRHI rhi;
#elif defined(RHI_DX12)
    DX12RHI rhi;
#elif defined(RHI_WEBGPU)
    WebGPURHI rhi;
#else
    #error No RHI backend selected.
#endif

    if (!rhi.Initialize())
        return 1;

    rhi.PrintCapabilities();
    rhi.Shutdown();

    std::cin.get();
    return 0;
}
