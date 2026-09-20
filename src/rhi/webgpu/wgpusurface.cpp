#include "rhi/webgpu/wgpurhi.h"

#include <iostream>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <third_party/glfw/include/GLFW/glfw3.h>
#ifdef _WIN32
#include <third_party/glfw/include/GLFW/glfw3native.h>
#include <windows.h>
#endif

rhi::SurfaceHandle WebGPURHI::CreateSurface(const rhi::SurfaceDesc& desc)
{
    if (!m_Instance || desc.window.id == 0xFFFFFFFFFFFFFFFF ||
        desc.window.id >= m_Windows.size() || m_Windows[desc.window.id] == nullptr) {
        return {};
    }

#ifndef _WIN32
    return {};
#else
    HWND hwnd = glfwGetWin32Window(m_Windows[desc.window.id]);
    if (hwnd == nullptr) {
        std::cerr << "Failed to get HWND from window.\n";
        return {};
    }

    wgpu::SurfaceSourceWindowsHWND nativeSource{};
    nativeSource.hinstance = GetModuleHandle(nullptr);
    nativeSource.hwnd = hwnd;

    wgpu::SurfaceDescriptor surfaceDesc{};
    surfaceDesc.nextInChain = &nativeSource;

    wgpu::Surface surface = m_Instance.CreateSurface(&surfaceDesc);
    if (!surface) {
        std::cerr << "Failed to create WebGPU surface.\n";
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
#endif
}

void WebGPURHI::DestroySurface(rhi::SurfaceHandle handle)
{
    if (!m_Instance || handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Surfaces.size() ||
        !m_Surfaces[handle.id]) {
        return;
    }
    m_Surfaces[handle.id] = {};
    m_FreeSurfaces.push_back(handle.id);
}
