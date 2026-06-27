#include <iostream>

#ifdef RHI_VULKAN
    #include "rhi/vulkan/vkrhi.h"
#elif defined(RHI_DX12)
    #include "rhi/dx12/dx12rhi.h"
#elif defined(RHI_WEBGPU)
    #include "rhi/webgpu/wgpurhi.h"
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

    rhi.Shutdown();

    std::cin.get();
    return 0;
}
