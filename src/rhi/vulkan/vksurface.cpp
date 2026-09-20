#include "rhi/vulkan/vkrhi.h"

#include <third_party/glfw/include/GLFW/glfw3.h>
#include <cstdio>

rhi::SurfaceHandle VulkanRHI::CreateSurface(const rhi::SurfaceDesc& desc)
{
    if (m_Instance == VK_NULL_HANDLE) {
        return {};
    }
    if (desc.window.id == 0xFFFFFFFFFFFFFFFF || desc.window.id >= m_Windows.size() ||
        m_Windows[desc.window.id] == nullptr) {
        return {};
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(m_Instance, m_Windows[desc.window.id], nullptr, &surface) !=
        VK_SUCCESS) {
        printf("[ERROR] Failed to create Vulkan surface\n");
        return {};
    }

    rhi::SurfaceHandle handle;
    handle.desc = desc;
    if (!m_FreeSurfaces.empty()) {
        handle.id = m_FreeSurfaces.back();
        m_FreeSurfaces.pop_back();
        m_Surfaces[handle.id] = surface;
    } else {
        handle.id = static_cast<std::uint64_t>(m_Surfaces.size());
        m_Surfaces.push_back(surface);
    }
    return handle;
}

void VulkanRHI::DestroySurface(rhi::SurfaceHandle handle)
{
    if (m_Instance == VK_NULL_HANDLE || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Surfaces.size() || m_Surfaces[handle.id] == VK_NULL_HANDLE) {
        return;
    }
    vkDestroySurfaceKHR(m_Instance, m_Surfaces[handle.id], nullptr);
    m_Surfaces[handle.id] = VK_NULL_HANDLE;
    m_FreeSurfaces.push_back(handle.id);
}
