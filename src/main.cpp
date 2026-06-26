#include <iostream>

#ifdef RHI_VULKAN
#include "rhi/vulkan/vkrhi.h"
#elif defined(RHI_DX12)
#include "rhi/dx12/dx12rhi.h"
#endif

#ifdef RHI_VULKAN
    VulkanRHI rhi;
#elif defined(RHI_DX12)
    DX12RHI rhi;
#else
#error No RHI backend selected.
#endif

int main()
{
    if (!rhi.Initialize())
        return 1;

    rhi.PrintCapabilities();
    rhi.Shutdown();

    std::cin.get();
}
