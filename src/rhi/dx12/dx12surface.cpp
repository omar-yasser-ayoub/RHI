#include "rhi/dx12/dx12rhi.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <third_party/glfw/include/GLFW/glfw3.h>
#include <third_party/glfw/include/GLFW/glfw3native.h>

rhi::SurfaceHandle DX12RHI::CreateSurface(const rhi::SurfaceDesc& desc)
{
    if (desc.window.id == 0xFFFFFFFFFFFFFFFF || desc.window.id >= m_Windows.size() ||
        m_Windows[desc.window.id] == nullptr) {
        return {};
    }

    HWND hwnd = glfwGetWin32Window(m_Windows[desc.window.id]);
    if (hwnd == nullptr) {
        return {};
    }

    rhi::SurfaceHandle handle;
    handle.desc = desc;
    if (!m_FreeSurfaces.empty()) {
        handle.id = m_FreeSurfaces.back();
        m_FreeSurfaces.pop_back();
        m_Surfaces[handle.id] = hwnd;
    } else {
        handle.id = static_cast<std::uint64_t>(m_Surfaces.size());
        m_Surfaces.push_back(hwnd);
    }
    return handle;
}

void DX12RHI::DestroySurface(rhi::SurfaceHandle handle)
{
    if (handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Surfaces.size() ||
        m_Surfaces[handle.id] == nullptr) {
        return;
    }
    m_Surfaces[handle.id] = nullptr;
    m_FreeSurfaces.push_back(handle.id);
}
