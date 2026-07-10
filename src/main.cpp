#include <iostream>

#ifdef RHI_VULKAN
    #include "rhi/vulkan/vkrhi.h"
    using RHI = VulkanRHI;
#elif defined(RHI_DX12)
    #include "rhi/dx12/dx12rhi.h"
    using RHI = DX12RHI;
#elif defined(RHI_WEBGPU)
    #include "rhi/webgpu/wgpurhi.h"
    using RHI = WebGPURHI;
#else
    #error No RHI backend selected.
#endif

int main()
{
    RHI rhi;

    if (!rhi.Initialize())
        return 1;

    rhi.Shutdown();

    std::cin.get();
    return 0;
}
