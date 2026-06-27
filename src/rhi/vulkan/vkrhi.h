#pragma once

#include "../IRHI.h"
#include <vulkan/vulkan.h>

class VulkanRHI : public IRHI
{
public:
    bool Initialize() override;
    void Shutdown() override;

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
};
